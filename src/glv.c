#include "_glv.h"

#define __DOC_CASE(MESSAGE, INPUT, OUTPUT, DESCRIPTION)\
    case MESSAGE:\
        glv_write_docs(\
        docs, MESSAGE,\
        SDL_STRINGIFY_ARG(MESSAGE),\
        INPUT, OUTPUT, DESCRIPTION); break;

static void create_mgr(GlvMgr *mgr);
static int delete_mgr(GlvMgr *mgr);
static void handle_events(GlvMgr *mgr, const SDL_Event *ev);
static void apply_events(GlvMgr *mgr);
static void unfocus_all_excepting(View *view, View *exception);
static bool handle_private_message(View *view, ViewMsg message, const void *in);
static unsigned int get_view_data_size(View *view);

static bool __init_sdl(GlvMgr *mgr);
static bool __init_window(GlvMgr *mgr);
static bool __init_gl_ctx(GlvMgr *mgr);
static bool __init_freetype2(GlvMgr *mgr);

static void __create_root_view(GlvMgr *mgr, ViewProc view_proc, ViewManage manage_proc, void *user_context);
static void __init_texture_1x1(View *view);
static void __init_framebuffer(View *view);
static bool __handle_winevents(GlvMgr *mgr, const SDL_WindowEvent *ev);
static void __handle_default_doc(ViewMsg msg, GlvMsgDocs *docs);
static void __draw_views_recursive(View *view, SDL_Rect parent);
static void __manage_default(View *view, ViewMsg msg, void *event_args, void *user_context);
static void __set_view_visibility(View *view, bool is_visible);
static void __set_is_mouse_over(View *view, bool is_mouse_over);
static void __unmap_child(View *child);

static void __enum_delete_childs(View *child, void *unused);
static void __enum_call_mouse_button_event(View *child, void *args_ptr);
static void __enum_call_mouse_move_event(View *child, void *args_ptr);
static void __enum_set_secondary_focus(View *view, void *unused);
static void __enum_call_key_event(View *view, void *args_ptr);
static void __enum_call_textinput_event(View *view, void *args_ptr);
static void __enum_call_textediting_event(View *view, void *args_ptr);
static void __enum_call_sdl_redirect_event(View *view, void *args_ptr);
static void __enum_call_mouse_wheel(View *view, void *args_ptr);
static void __enum_draw_views(View *view, void *unused);
static void __enum_point_to_parent(View *view, void *args);
static void __enum_point_to_child(View *view, void *args);

static void __handle_mouse_button(View *root, ViewMsg msg, const SDL_MouseButtonEvent *ev);
static void __handle_mouse_move(View *root, const SDL_MouseMotionEvent *ev);
static void __handle_key(View *root, ViewMsg msg, const SDL_KeyboardEvent *ev);

static void *__create_singleton_data_ifnexists(GlvMgr *mgr, ViewProc proc);
static void __delete_singletons(GlvMgr *mgr);
static Uint32 __get_singleton_data_size(ViewProc proc);

static Uint32 user_msg_first;

void glv_enum_childs(View *view, void(*enum_proc)(View *childs, void *data), void *data){
    SDL_assert(view != NULL);
    SDL_assert(enum_proc != NULL);

    View **curr = view->childs;
    View **end = curr + view->childs_cnt;

    while (curr != end){
        enum_proc(*curr, data);
        curr++;
    }
}

void glv_enum_visible_childs(View *view, void(*enum_proc)(View *childs, void *data), void *data){
    SDL_assert(view != NULL);
    SDL_assert(enum_proc != NULL);
    
    View **curr = view->childs;
    View **end = curr + view->childs_cnt;

    while (curr != end){
        if(curr[0]->is_visible){
            enum_proc(*curr, data);
        }
        curr++;
    }
}

void glv_enum_focused_childs(View *view, void(*enum_proc)(View *childs, void *data), void *data){
    SDL_assert(view != NULL);

    View **curr = view->childs;
    View **end = curr + view->childs_cnt;

    while (curr != end){
        if(curr[0]->is_focused){
            enum_proc(*curr, data);
        }
        curr++;
    }
}

void glv_enum_hovered_childs(View *view, void(*enum_proc)(View *childs, void *data), void *data){
    SDL_assert(view != NULL);

    View **curr = view->childs;
    View **end = curr + view->childs_cnt;

    while (curr != end){
        if(curr[0]->is_mouse_over){
            enum_proc(*curr, data);
        }
        curr++;
    }
}

void glv_enum_parents(View *view, void(*enum_proc)(View *parent, void *data), void *data){
    if(view == NULL) return;
    enum_proc(view->parent, data);
    glv_enum_parents(view->parent, enum_proc, data);
}

View *glv_create(View *parent, ViewProc view_proc, ViewManage manage_proc, void *user_context){
    SDL_assert(parent != NULL);
    SDL_assert(view_proc != NULL);


    View *result = malloc(sizeof(View));
    SDL_zerop(result);

    result->mgr = parent->mgr;
    result->parent = parent;
    result->view_proc = view_proc;
    result->user_context = user_context;
    result->is_visible = true;
    result->singleton_data = __create_singleton_data_ifnexists(parent->mgr, view_proc);

    if(manage_proc == NULL) result->view_manage = __manage_default;
    else result->view_manage = manage_proc;

    __init_texture_1x1(result);
    __init_framebuffer(result);

    parent->childs_cnt++;
    parent->childs = realloc(parent->childs, parent->childs_cnt * sizeof(View*));
    parent->childs[parent->childs_cnt - 1] = result;

    result->view_data_size = get_view_data_size(result);
    if(result->view_data_size == 0) result->view_data = NULL;
    else result->view_data = malloc(result->view_data_size);
    memset(result->view_data, 0, result->view_data_size);

    glv_call_event(result, VM_CREATE, NULL, NULL);
    glv_call_manage(result, VM_CREATE, NULL);

    GlvChildChanged args = {
        .child = result,
        .sender = NULL,
    };
    glv_push_event(parent, VM_CHILD_CREATE, &args, sizeof(args));

    return result;
}

void glv_delete(View *view){
    if(view == NULL) return;
    if(view == view->mgr->root_view){
        view->mgr->root_view = NULL;
        view->mgr->is_running = false;
    }
    else{
        GlvChildChanged args = {
            .child = view,
            .sender = NULL,
        };
        glv_push_event(view->parent, VM_CHILD_DELETE, &args, sizeof(args));
    }

    glDeleteTextures(1, &view->bg_tex);
    glDeleteTextures(1, &view->fg_tex);

    glv_enum_childs(view, __enum_delete_childs, NULL);
    glv_push_event(view, VM_DELETE, NULL, 0);

    glv_push_event(view, VM_VIEW_FREE__, NULL, 0);
}

GlvMgr *glv_get_mgr(View *view){
    SDL_assert(view != NULL);

    return view->mgr;
}

void glv_proc_default(View *view, ViewMsg msg, void *in, void *out){
    in = in;//unused
    out = out;//unused
    view = view;//unused
    switch (msg){
    case VM_GET_DOCS:
        __handle_default_doc(*(ViewMsg*)in, out);
        break;
    }
}

int glv_run(ViewProc root_view_proc, ViewManage root_view_manage, void *root_user_data, void (*init_spa)(View *root_view, void *root_context)){
    SDL_assert(root_view_proc != NULL);
    SDL_assert(init_spa != NULL);

    GlvMgr mgr;
    create_mgr(&mgr);
    mgr.is_running = true;

    if(!__init_sdl(&mgr)
    || !__init_window(&mgr)
    || !__init_gl_ctx(&mgr)
    || !__init_freetype2(&mgr)){
        return EXIT_FAILURE;
    }

    SDL_DisplayMode window_display_mode;
    SDL_GetWindowDisplayMode(mgr.window, &window_display_mode);
    glv_set_minimal_frametime(&mgr, 1000 / window_display_mode.refresh_rate);

    mgr.draw_texture_program = init_draw_texture(&mgr);
    mgr.draw_circle_program = init_draw_circle(&mgr);
    mgr.draw_triangle_program = init_draw_triangle(&mgr);
    mgr.draw_text_program = init_draw_text(&mgr);

    __create_root_view(&mgr, root_view_proc, root_view_manage, root_user_data);


    init_spa(mgr.root_view, root_user_data);

    SDL_ShowWindow(mgr.window);
    SDL_Event ev;
    while (mgr.is_running){
        while(SDL_PollEvent(&ev)){
            handle_events(&mgr, &ev);
        }

        apply_events(&mgr);
        SDL_Delay(mgr.min_frametime_ms);
    }

    if(mgr.root_view != NULL){
        glv_delete(mgr.root_view);
    }

    while(SDL_PollEvent(&ev)){
        handle_events(&mgr, &ev);
    }

    return delete_mgr(&mgr);
}

void *glv_get_view_data(View *view, unsigned int offset){
    SDL_assert(view != NULL);

    if(view->view_data_size <= offset){
        char err_log[256+1];
        snprintf(err_log, 256, "glv_get_view_data with offset out of data range, offset:%i", offset);
        glv_log_err(glv_get_mgr(view), err_log);
        glv_quit(glv_get_mgr(view));
        return NULL;
    }
    else{
        return (char*)view->view_data + offset;
    }
}

void *glv_get_view_singleton_data(View *view){
    SDL_assert(view != NULL);

    return view->singleton_data;
}

void glv_push_event(View *view, ViewMsg message, void *args, uint32_t args_size){
    SDL_assert(view != NULL);
    SDL_assert((args_size != 0 && args == NULL) == false);

    SDL_Event ev;
    GlvSdlEvent *glv_ev = (GlvSdlEvent*)&ev.user;
    glv_ev->type = SDL_USEREVENT;
    glv_ev->message = message + user_msg_first;
    glv_ev->view = view;
    glv_ev->windowID = view->mgr->wind_id;
    glv_ev->timestamp = SDL_GetTicks();
    
    if(args_size == 0){
        glv_ev->data = NULL;
    }
    else{
        glv_ev->data = malloc(args_size);
        memcpy(glv_ev->data, args, args_size);
    }

    SDL_PushEvent(&ev);
}

void glv_call_event(View *view, ViewMsg message, void *in, void *out){
    SDL_assert(view != NULL);

    view->view_proc(view, message, in, out);
}

void glv_call_manage(View *view, ViewMsg message, void *event_args){
    SDL_assert(view != NULL);

    view->view_manage(view, message, event_args, view->user_context);
}

GLuint glv_get_texture(View *view){
    SDL_assert(view != NULL);

    return view->texture;
}

GLuint glv_get_framebuffer(View *view){
    SDL_assert(view != NULL);

    return view->framebuffer;
}

GlvMsgDocs glv_get_docs(View *view, ViewMsg message){
    GlvMsgDocs result;
    SDL_zero(result);
    result.msg = VM_NULL;

    if(view == NULL){
        if(message == VM_GET_SINGLETON_DATA_SIZE){
            glv_write_docs(&result, message, SDL_STRINGIFY_ARG(VM_GET_SINGLETON_DATA_SIZE), 
            "NULL", "NULL", "uses to initialize singleton data, view is NULL");
        }
        else if(message == VM_SINGLETON_DATA_DELETE){
            glv_write_docs(&result, message, SDL_STRINGIFY_ARG(VM_SINGLETON_DATA_DELETE), 
            "void *singleton_data", "NULL", "uses to delete singleton data contents, view is NULL");
        }
        else if(message == VM_VIEW_FREE__){
            glv_write_docs(&result, message, SDL_STRINGIFY_ARG(VM_VIEW_FREE__), 
            "NULL", "NULL", "uses to delete view, doesnt sends to view or manage proc, view is NULL");
        }
        else{
            glv_write_docs(&result, message, "undefined", 
            "undefined", "undefined", "undefined, view is NULL");
        }
        return result;
    }

    if(message == VM_GET_DOCS){
        result.name = SDL_STRINGIFY_ARG(VM_GET_DOCS);
        result.input_description = "const ViewMsg *message";
        result.output_description = "GlvMsgDocs *message_docs";
        result.description = "describes passed message";
    }
    else{
        view->view_proc(view, VM_GET_DOCS, &message, &result);
        if(result.msg == VM_NULL){
            result.msg = message;
            result.name = "undefined";
            result.input_description = "undefined";
            result.output_description = "undefined";
            result.description = "undefined";
        }
        else{
            if(result.name == NULL) result.name = "undefined";
            if(result.input_description == NULL) result.input_description = "undefined";
            if(result.output_description == NULL) result.output_description = "undefined";
            if(result.description == NULL) result.description = "undefined";
        }
    }
    
    result.msg = message;
    return result;
}

void glv_write_docs(GlvMsgDocs *docs, ViewMsg message, 
    const char *name, const char *input_description, 
    const char *output_description, const char *general_description){
    
    SDL_assert(docs != NULL);

    docs->msg = message;
    docs->name = name;
    docs->input_description = input_description;
    docs->output_description = output_description;
    docs->description = general_description;
}

void glv_set_minimal_frametime(GlvMgr *mgr, Uint32 ms_min_frametime){
    SDL_assert(mgr);

    mgr->min_frametime_ms = ms_min_frametime;
}

void glv_set_focus(View *view){
    SDL_assert(view != NULL);

    unfocus_all_excepting(view, view);
    glv_set_secondary_focus(view);
    glv_enum_parents(view, __enum_set_secondary_focus, NULL);
}

//uses to handle recursion when sended by parent view while handling CHILD_SHOW event
void glv_show_by(View *sender, View *view){
    SDL_assert(view != NULL);

    __set_view_visibility(view, true);
    glv_push_event(view, VM_SHOW, NULL, 0);
    if(view->parent != NULL){
        GlvChildChanged args = {
            .child = view,
            .sender = sender,
        };
        glv_push_event(view->parent, VM_CHILD_SHOW, &args, sizeof(args));
    }
}

//uses to handle recursion when sended by parent view while handling CHILD_HIDE event
void glv_hide_by(View *sender, View *view){
    SDL_assert(view != NULL);

    __set_view_visibility(view, false);
    glv_push_event(view, VM_HIDE, NULL, 0);
    if(view->parent != NULL){
        GlvChildChanged args = {
            .child = view,
            .sender = sender,
        };
        glv_push_event(view->parent, VM_CHILD_HIDE, &args, sizeof(args));
    }
    unfocus_all_excepting(view, NULL);

    __set_is_mouse_over(view, false);
}

//uses to handle recursion when sended by parent view while handling CHILD_MOVE event
void glv_set_pos_by(View *sender, View *view, int x, int y){
    SDL_assert(view != NULL);

    if(view == view->mgr->root_view){
        SDL_SetWindowPosition(view->mgr->window, x, y);
    }
    else{
        view->x = x;
        view->y = y;

        SDL_Point new_pos = {
            .x = x,
            .y = y,
        };
        glv_push_event(view, VM_MOVE, &new_pos, sizeof(new_pos));
        GlvChildChanged args = {
            .child = view,
            .sender = sender,
        };
        glv_push_event(view->parent, VM_CHILD_MOVE, &args, sizeof(args));
        glv_redraw_window(view->mgr);
    }
}

//uses to handle recursion when sended by parent view while handling CHILD_RESIZE event
void glv_set_size_by(View *sender, View *view, unsigned int width, unsigned int height){
    SDL_assert(view != NULL);

    if(view == view->mgr->root_view){
        SDL_SetWindowSize(view->mgr->window, width, height);
    }
    else{
        view->w = width;
        view->h = height;
        SDL_Point new_size = {
            .x = width,
            .y = height,
        };
        glv_push_event(view, VM_RESIZE, &new_size, sizeof(new_size));
        GlvChildChanged args = {
            .child = view,
            .sender = sender,
        };
        glv_push_event(view->parent, VM_CHILD_RESIZE, &args, sizeof(args));
        glv_redraw_window(view->mgr);
    }
}

SDL_Point glv_get_pos(View *view){
    SDL_assert(view != NULL);
    if(view->mgr->root_view == view){
        SDL_Point result;
        SDL_GetWindowPosition(view->mgr->window, &result.x, &result.y);
        return result;
    }else return (SDL_Point){
        .x = view->x,
        .y = view->y,
    };
}

SDL_Point glv_get_size(View *view){
    SDL_assert(view != NULL);
    if(view->mgr->root_view == view){
        SDL_Point result;
        SDL_GetWindowSize(view->mgr->window, &result.x, &result.y);
        return result;
    }else return (SDL_Point){
        .x = view->w,
        .y = view->h,
    };
}

View *glv_get_Parent(View *view){
    SDL_assert(view != NULL);
    return view->parent;
}

//if view is root, returns desktop position 
SDL_Point glv_view_to_parent(View *view, SDL_Point point){
    SDL_assert(view != NULL);

    if(view->parent != NULL){
        return (SDL_Point){
            .x = point.x + view->x,
            .y = point.y + view->y,
        };
    }
    else{
        return glv_window_to_desktop(view->mgr, point);
    }
}

SDL_Point glv_view_to_window(View *view, SDL_Point point){
    SDL_assert(view != NULL);

    glv_enum_parents(view, __enum_point_to_parent, &point);    
    return point;
}


SDL_Point glv_window_to_view(View *view, SDL_Point point){
    SDL_assert(view != NULL);

    glv_enum_parents(view, __enum_point_to_child, &point);
    return point; 
}

SDL_Point glv_window_to_desktop(GlvMgr *mgr, SDL_Point point){
    SDL_assert(mgr != NULL);

    int x, y;
    SDL_GetWindowPosition(mgr->window, &x, &y);

    point.x += x;
    point.y += y;

    return point;
}

SDL_Point glv_desktop_to_window(GlvMgr *mgr, SDL_Point point){
    SDL_assert(mgr != NULL);

    int x, y;
    SDL_GetWindowPosition(mgr->window, &x, &y);

    point.x -= x;
    point.y -= y;

    return point;
}

bool glv_is_focused(View *view){
    SDL_assert(view != NULL);
    return view->is_focused;
}

bool glv_is_visible(View *view){
    SDL_assert(view != NULL);
    
    return view->is_visible;
}

bool glv_is_mouse_over(View *view){
    SDL_assert(view != NULL);

    return view->is_mouse_over;
}

bool glv_build_program_or_quit_err(GlvMgr *mgr, const char *vertex, const char *fragment, GLuint *result){
    SDL_assert(mgr != NULL);
    SDL_assert(vertex != NULL);
    SDL_assert(fragment != NULL);
    SDL_assert(result != NULL);

    GLuint f = glCreateShader(GL_FRAGMENT_SHADER);
    GLuint v = glCreateShader(GL_VERTEX_SHADER);

    glShaderSource(f, 1, (const GLchar**)&fragment, NULL);
    glShaderSource(v, 1, (const GLchar**)&vertex, NULL);

    glCompileShader(f);
    glCompileShader(v);

    GLuint prog = glCreateProgram();
    glAttachShader(prog, f);
    glAttachShader(prog, v);

    glLinkProgram(prog);

    glDeleteShader(f);
    glDeleteShader(v);

    GLint status;
    glGetProgramiv(prog, GL_LINK_STATUS, &status);
    if(status == 0){
        char buffer[128];
        glGetProgramInfoLog(prog, sizeof(buffer) - 1, NULL, buffer);
        glv_log_err(mgr, buffer);
        glDeleteProgram(prog);

        *result = 0;

        return false;
    }
    else{
        *result = prog;
        return true;
    }
}

void glv_set_secondary_focus(View *view){
    
    if(view == NULL) return;
    if(view->is_focused != false) return;

    glv_show(view);
    
    if(view->mgr->root_view == view){
        SDL_SetWindowInputFocus(view->mgr->window);
    }
    else{
        view->is_focused = true;
        glv_push_event(view, VM_FOCUS, NULL, 0);
    }
}

void glv_unset_secondary_focus(View *view){
    SDL_assert(view != NULL);

    unfocus_all_excepting(view, NULL);
}

GLuint glv_swap_texture(View *view, GLuint texture){
    SDL_assert(view != NULL);

    GLuint prev = view->texture;
    view->texture = texture;

    glBindFramebuffer(GL_FRAMEBUFFER, view->framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_BUFFER_BIT, GL_TEXTURE_2D, texture, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    view->is_drawable = true;
    glv_redraw_window(view->mgr);

    return prev;
}

void glv_swap_texture_with_bg(View *view){
    SDL_assert(view != NULL);

    glBindFramebuffer(GL_FRAMEBUFFER, view->framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_BUFFER_BIT, GL_TEXTURE_2D, view->bg_tex, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    GLuint tmp = view->texture;
    view->texture = view->bg_tex;
    view->bg_tex = tmp;

    view->is_drawable = true;
    glv_redraw_window(view->mgr);
}

void glv_set_background(View *view, GLuint texture){
    SDL_assert(view != NULL);

    glDeleteTextures(1, &view->bg_tex);
    view->bg_tex = texture;

    glv_push_event(view, VM_SET_BG, &texture, sizeof(texture));
}


void glv_set_foreground(View *view, GLuint texture){
    SDL_assert(view != NULL);

    glDeleteTextures(1, &view->fg_tex);
    view->fg_tex = texture;

    glv_push_event(view, VM_SET_FG, &texture, sizeof(texture));
}

GLuint glv_get_bg_texture(View *view){
    SDL_assert(view != NULL);

    return view->bg_tex;
}

GLuint glv_get_fg_texture(View *view){
    SDL_assert(view != NULL);

    return view->fg_tex;
}

GLuint glv_gen_texture_solid_color(Uint8 r, Uint8 g, Uint8 b, Uint8 a){
    SDL_Color color = {
        .r = r,
        .g = g,
        .b = b,
        .a = a,
    };

    GLuint result;
    glGenTextures(1, &result);
    glBindTexture(GL_TEXTURE_2D, result);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &color);

    glBindTexture(GL_TEXTURE_2D, 0);
    return result;
}

void glv_set_font(View *view, GlvFontFaceId font_face){
    SDL_assert(view != NULL);

    glv_push_event(view, VM_SET_FONT, &font_face, sizeof(font_face));
}

void glv_set_font_width(View *view, Uint32 font_width){
    SDL_assert(view != NULL);

    glv_push_event(view, VM_SET_FONT_WIDTH, &font_width, sizeof(font_width));
}

void glv_set_font_height(View *view, Uint32 font_height){
    SDL_assert(view != NULL);

    glv_push_event(view, VM_SET_FONT_HEIGHT, &font_height, sizeof(font_height));
}

void glv_print_docs(View *view, ViewMsg message){
    GlvMsgDocs docs = glv_get_docs(view, message);

    printf("\033[0;32m");
    printf("message(%i):\t%s\n", docs.msg, docs.name);
    printf("    input:\t%s\n", docs.input_description);
    printf("    output:\t%s\n", docs.output_description);
    printf("    general:\n%s\n", docs.description);
    printf("\033[0m\n");
}


void glv_set_pos(View *view, int x, int y){
    glv_set_pos_by(NULL, view, x, y);
}

void glv_set_size(View *view, unsigned int width, unsigned int height){
    glv_set_size_by(NULL, view, width, height);
}

void glv_draw(View *view){
    SDL_assert(view != NULL);

    view->is_drawable = true;
    view->is_redraw_queue = true;
    
    glv_redraw_window(view->mgr);
}

void glv_draw_views_recursive(View *view){
    SDL_Rect parent = {
        .x = 0,
        .y = 0,
        .w = view->w,
        .h = view->h
    };
    
    __draw_views_recursive(view, parent);
}

void glv_force_draw_views_recursive(View *view){
    __enum_draw_views(view, NULL);

    glBindFramebuffer(GL_FRAMEBUFFER, view->framebuffer);
    glv_draw_views_recursive(view);
}

void glv_deny_draw(View *view){
    SDL_assert(view != NULL);

    view->is_drawable = false;
    glv_redraw_window(view->mgr);
}

void glv_show(View *view){
    glv_show_by(NULL, view);
}

void glv_hide(View *view){
    glv_hide_by(NULL, view);
}

static void create_mgr(GlvMgr *mgr){
    SDL_zerop(mgr);
    mgr->logger_proc = log_printf;
    mgr->min_frametime_ms = 10;
    mgr->last_frame_drawed_time_ms = SDL_GetTicks();
    mgr->on_sdl_event = default_on_sdl_event;
}

static int delete_mgr(GlvMgr *mgr){
    __delete_singletons(mgr);

    mgr->faces_cnt = 0;
    free(mgr->faces);
    mgr->faces = NULL;

    FT_Done_FreeType(mgr->ft_lib);

    free_draw_texture(&mgr->draw_texture_program);
    free_draw_circle(&mgr->draw_circle_program);
    free_draw_triangle(&mgr->draw_triangle_program);
    free_draw_text(&mgr->draw_text_program);

    SDL_DestroyWindow(mgr->window);
    SDL_GL_DeleteContext(mgr);
    return mgr->return_code;
}

static void handle_events(GlvMgr *mgr, const SDL_Event *ev){
    if(mgr->root_view != NULL){
        mgr->on_sdl_event(mgr->root_view, ev, mgr->root_view->user_context);
    }
    else{
        return;
    }
    
    switch (ev->type){
        case SDL_WINDOWEVENT:
            if(__handle_winevents(mgr, &ev->window)) return;
            else break;
        case SDL_QUIT:{
            mgr->is_running = false;
        }return;
        case SDL_USEREVENT:{
            if(ev->user.code < (Sint32)user_msg_first || ev->user.code > (Sint32)user_msg_first + VM_USER_LAST) break;
            const GlvSdlEvent *glv_ev = (const GlvSdlEvent*)&ev->user;
            if(!handle_private_message(glv_ev->view, glv_ev->message - user_msg_first, glv_ev->data)){
                glv_call_event(glv_ev->view, glv_ev->message - user_msg_first, glv_ev->data, NULL);
                glv_call_manage(glv_ev->view, glv_ev->message - user_msg_first, glv_ev->data);
            }
            free(glv_ev->data);
        } return;
        case SDL_MOUSEWHEEL:{
            if(ev->wheel.windowID != mgr->wind_id) return;
            GlvWheel wheel = {
                .direction = ev->wheel.direction,
                .which = ev->wheel.which,
                .preciseX = ev->wheel.preciseX,
                .preciseY = ev->wheel.preciseY,
            };
            __enum_call_mouse_wheel(mgr->root_view, &wheel);
        } return;
        case SDL_MOUSEBUTTONDOWN:
            if(ev->button.windowID != mgr->wind_id) return;
            __handle_mouse_button(mgr->root_view, VM_MOUSE_DOWN, &ev->button);
        return;
        case SDL_MOUSEBUTTONUP:
            if(ev->button.windowID != mgr->wind_id) return;
            __handle_mouse_button(mgr->root_view, VM_MOUSE_UP, &ev->button);
        return;
        case SDL_MOUSEMOTION:{
            if(ev->motion.windowID != mgr->wind_id) return;
            __handle_mouse_move(mgr->root_view, &ev->motion);
        }return;
        case SDL_KEYDOWN:{
            if(ev->key.windowID != mgr->wind_id) return;
            __handle_key(mgr->root_view, VM_KEY_DOWN, &ev->key);
        }return;
        case SDL_KEYUP:{
            if(ev->key.windowID != mgr->wind_id) return;
            __handle_key(mgr->root_view, VM_KEY_UP, &ev->key);
        }return;
        case SDL_TEXTINPUT:{
            if(ev->text.windowID) return;
            __enum_call_textinput_event(mgr->root_view, (char*)&(ev->text.text[0]));
        }return;
        case SDL_TEXTEDITING:{
            if(ev->edit.windowID) return;
            GlvTextEditing te;
            memcpy(te.composition, ev->edit.text, sizeof(char[32]));
            te.cursor = ev->edit.start;
            te.selection_len = ev->edit.length;
            __enum_call_textediting_event(mgr->root_view, &te);
        }return;
    }
    if(mgr->root_view != NULL){
        __enum_call_sdl_redirect_event(mgr->root_view, (void*)ev);
    }
}

static void apply_events(GlvMgr *mgr){
    if(should_redraw(mgr)){
        if(mgr->root_view == NULL){
            glv_log_err(mgr, "redrawing NULL View");
            return;
        }
    
        __enum_draw_views(mgr->root_view, NULL);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, mgr->root_view->w, mgr->root_view->h);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glv_draw_views_recursive(mgr->root_view);
        SDL_GL_SwapWindow(mgr->window);
    }
}

static void unfocus_all_excepting(View *view, View *exception){
    if(view->is_focused == false) return;
    
    if(view != exception){
        view->is_focused = false;
        glv_push_event(view, VM_UNFOCUS, NULL, 0);
    }
   
    glv_enum_childs(view, (void(*)(View *, void*))unfocus_all_excepting, exception);
}

static bool handle_private_message(View *view, ViewMsg message, const void *in){
    in = in;//unused
    switch (message){
    case VM_VIEW_FREE__:
        __unmap_child(view);
        free(view->view_data);
        free(view);
        return true;
    default: return false;
    }
}

static unsigned int get_view_data_size(View *view){
    Uint32 data_size = 0;
    glv_call_event(view, VM_GET_VIEW_DATA_SIZE, NULL, &data_size);
    return data_size;
}

static bool __init_sdl(GlvMgr *mgr){
    if(SDL_Init(SDL_INIT_VIDEO) < 0){
        glv_log_err(mgr, SDL_GetError());
        user_msg_first = SDL_RegisterEvents(VM_USER_LAST + 1);
        return false;
    }
    else{
        return true;
    }
}

static bool __init_window(GlvMgr *mgr){
    mgr->window = SDL_CreateWindow("", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, 
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL);
    if(mgr->window == NULL){
        glv_log_err(mgr, "cannot create SDL2_Window");
        return false;
    }
    else{
        mgr->wind_id = SDL_GetWindowID(mgr->window);
    }
    return true;
}

static bool __init_gl_ctx(GlvMgr *mgr){
    SDL_assert(mgr->window != NULL);
    mgr->gl_rc = SDL_GL_CreateContext(mgr->window);
    if(mgr->gl_rc == NULL){
        glv_log_err(mgr, "cannot create SDL_GlContext");
        return false;
    }
    SDL_GL_MakeCurrent(mgr->window, mgr->gl_rc);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    return true;
}

static bool __init_freetype2(GlvMgr *mgr){
    FT_Error err = FT_Init_FreeType(&mgr->ft_lib);
    if(err != FT_Err_Ok){
        glv_log_err(mgr, "cannot load freetype2 library");
        glv_log_err(mgr, FT_Error_String(err));
        return false;
    }
    return true;
}

static void __create_root_view(GlvMgr *mgr, ViewProc view_proc, ViewManage manage_proc, void *user_context){
    View *result = malloc(sizeof(View));
    SDL_zerop(result);
    mgr->root_view = result;

    result->mgr = mgr;
    result->view_proc = view_proc;
    result->user_context = user_context;
    result->is_visible = true;
    result->singleton_data = __create_singleton_data_ifnexists(mgr, view_proc);

    if(manage_proc == NULL) result->view_manage = __manage_default;
    else result->view_manage = manage_proc;
    
    __init_texture_1x1(result);
    __init_framebuffer(result);

    result->view_data_size = get_view_data_size(result);
    if(result->view_data_size == 0) result->view_data = NULL;
    else result->view_data = malloc(result->view_data_size);
    memset(result->view_data, 0, result->view_data_size);

    glv_call_event(result, VM_CREATE, NULL, NULL);
    glv_call_manage(result, VM_CREATE, NULL);
}

static void __init_texture_1x1(View *view){
    glGenTextures(1, &view->texture);
    glBindTexture(GL_TEXTURE_2D, view->texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    SDL_Color default_pixel = {.r = 255, .g = 255,.b = 255, .a = 255};

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &default_pixel);

    glBindTexture(GL_TEXTURE_2D, 0);
}

static void __init_framebuffer(View *view){
    glGenFramebuffers(1, &view->framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, view->framebuffer);
    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, view->texture, 0);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
        glv_log_err(view->mgr, "cannot create framebuffer for view");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


static bool __handle_winevents(GlvMgr *mgr, const SDL_WindowEvent *ev){
    if(mgr->root_view == NULL){
        glv_log_err(mgr, "root view is NULL");
        return true;
    }
    else if(ev->windowID != mgr->wind_id) return false;

    switch (ev->event){
    case SDL_WINDOWEVENT_EXPOSED:
        mgr->required_redraw = true;
        return true;
    case SDL_WINDOWEVENT_SIZE_CHANGED:{
        SDL_Point new_size = {
            .x = mgr->root_view->w = ev->data1,
            .y = mgr->root_view->h = ev->data2,
        };

        glv_push_event(mgr->root_view, VM_RESIZE, &new_size, sizeof(new_size));
    }return true;
    case SDL_WINDOWEVENT_SHOWN:{
        SDL_Point new_pos;
        SDL_Point new_size;

        mgr->root_view->is_visible = true;
        glv_push_event(mgr->root_view, VM_SHOW, NULL, 0);

        SDL_GetWindowSize(mgr->window, &new_size.x, &new_size.y);
        SDL_GetWindowPosition(mgr->window, &new_pos.x, &new_pos.y);

        glv_push_event(mgr->root_view, VM_MOVE, &new_size, sizeof(new_pos));
        glv_push_event(mgr->root_view, VM_RESIZE, &new_size, sizeof(new_size));
    }return true;
    case SDL_WINDOWEVENT_HIDDEN:{
        mgr->root_view->is_visible = false;
        glv_push_event(mgr->root_view, VM_HIDE, NULL, 0);
    }return true;
    case SDL_WINDOWEVENT_FOCUS_GAINED:{
        mgr->root_view->is_focused = true;
        glv_push_event(mgr->root_view, VM_FOCUS, NULL, 0);
    }return true;
    case SDL_WINDOWEVENT_FOCUS_LOST:{
        mgr->root_view->is_focused = false;
        glv_push_event(mgr->root_view, VM_UNFOCUS, NULL, 0);
    }return true;
    }
    return false;
}

static void __handle_default_doc(ViewMsg msg, GlvMsgDocs *docs){
    switch (msg){
        __DOC_CASE(VM_NULL, "NULL", "NULL", "doesnt handles");
        __DOC_CASE(VM_VIEW_FREE__, "NULL", "NULL", "handles only by view manager");
        __DOC_CASE(VM_CREATE, "NULL", "NULL", "calls once on create, handles without queue");
        __DOC_CASE(VM_DELETE, "NULL", "NULL", "calls once on delete");
        __DOC_CASE(VM_RESIZE, "const SDL_Point *new_size", "NULL", "calls when view is resized");
        __DOC_CASE(VM_MOVE, "const SDL_Point *new_pos", "NULL", "calls when view is moved");
        __DOC_CASE(VM_MOUSE_DOWN, "const GlvMouseDown *args", "NULL", "calls when is mouse pressed");
        __DOC_CASE(VM_MOUSE_UP, "const GlvMouseUp *args", "NULL", "calls when is mouse released");
        __DOC_CASE(VM_MOUSE_MOVE, "const GlvMouseMove *args", "NULL", "calls when is mouse moved");
        __DOC_CASE(VM_MOUSE_WHEEL, "const GlvWheel *args", "NULL", "calls on mouse wheel");
        __DOC_CASE(VM_DRAW, "NULL", "NULL", "calls when glv_draw(view) is called");
        __DOC_CASE(VM_SHOW, "NULL", "NULL", "calls on show");
        __DOC_CASE(VM_HIDE, "NULL", "NULL", "calls on hide");
        __DOC_CASE(VM_FOCUS, "NULL", "NULL", "calls on focus");
        __DOC_CASE(VM_UNFOCUS, "NULL", "NULL", "calls on focus lost");
        __DOC_CASE(VM_KEY_DOWN, "const GlvKeyDown *args", "NULL", "calls on key down and view is focused");
        __DOC_CASE(VM_KEY_UP, "const GlvKeyUp *args", "NULL", "calls on key up and view is focused");
        __DOC_CASE(VM_CHILD_RESIZE, "GlvChildChanged* view", "NULL", "calls on child resize");
        __DOC_CASE(VM_CHILD_MOVE, "GlvChildChanged* view", "NULL", "calls on c__enum_call_sdl_redirect_eventhild move");
        __DOC_CASE(VM_CHILD_CREATE, "GlvChildChanged* view", "NULL", "calls on child create");
        __DOC_CASE(VM_CHILD_DELETE, "GlvChildChanged* view", "NULL", "calls on child delete");
        __DOC_CASE(VM_CHILD_HIDE, "GlvChildChanged* view", "NULL", "calls on child hide");
        __DOC_CASE(VM_CHILD_SHOW, "View* view", "NULL", "calls on child show");
        __DOC_CASE(VM_MOUSE_HOVER, "NULL", "NULL", "calls on mouse hover");
        __DOC_CASE(VM_MOUSE_LEAVE, "NULL", "NULL", "calls on mouse leave");
        __DOC_CASE(VM_TEXT, "const char *", "NULL", "calls on text input if glv_is_focused(view)");
        __DOC_CASE(VM_TEXT_EDITING, "const GlvTextEditing *args", "NULL", "redirect of SDL_TEXTEDITING, requires SDL_StartTextInput");
        __DOC_CASE(VM_SDL_REDIRECT, "const SDL_Event *args", "NULL", "redirect of sdl events, handles without queue");
        __DOC_CASE(VM_SET_BG, "const GLuint *background_texture", "NULL", "called after setting background, input parameter is already bounded to view, view should manage redrawing manually");
        __DOC_CASE(VM_SET_FG, "const GLuint *foreground_texture", "NULL", "called after setting foreground, input parameter is already bounded to view, view should manage redrawing manually");
        __DOC_CASE(VM_SET_FONT, "const GlvFontFaceId *face_id", "NULL", "called after setting face input, view should manage redrawing manually");
        __DOC_CASE(VM_SET_FONT_WIDTH, "const Uint32 *font width", "NULL", "called after setting font width, view should manage redrawing manually");
        __DOC_CASE(VM_SET_FONT_HEIGHT, "const Uint32 *font height", "NULL", "called after setting font heaight, view should manage redrawing manually");
        __DOC_CASE(VM_GET_DOCS, "const ViewMsg *message", "GlvMsgDocs *docs", "called on glv_get_docs or glv_print_docs, handles without queue");
        __DOC_CASE(VM_GET_VIEW_DATA_SIZE, "NULL", "Uint32 *data_size", "called after CREATE to get view extra data size, data can be used via get_view_data, handles without queue");
        __DOC_CASE(VM_GET_SINGLETON_DATA_SIZE, "NULL", "Uint32 *data_size", "called once before first same view create to init data shared for same view data, View *view == NULL for this event, handles without queue");
        __DOC_CASE(VM_SINGLETON_DATA_DELETE, "void *singleton_data", "NULL", "called once right before singletond data is destroyed, View *view == NULL for this event, handles without queue");
    }
}

static void __draw_views_recursive(View *view, SDL_Rect parent){
    if(view->is_visible == false) return;

    SDL_Rect dst;
    SDL_Rect src;

    int availabale_w = parent.w - view->x;
    int availabale_h = parent.h - view->y;

    if(availabale_w <= 0 || availabale_h <= 0) return;

    dst.x = parent.x + view->x;
    dst.y = parent.y + view->y;
    dst.w = SDL_min(availabale_w, view->w);
    dst.h = SDL_min(availabale_h, view->h);

    if(view->is_drawable){
        src.x = 0;
        src.y = 0;
        src.w = view->w;
        src.h = view->h;

        glv_draw_texture_absolute(view->mgr, view->texture, &src, &dst);
    }

    View **curr = view->childs;
    View **end = curr + view->childs_cnt;

    while (curr !=end){
        __draw_views_recursive(*curr, dst);
        curr++;
    }
}

static void __manage_default(View *view, ViewMsg msg, void *event_args, void *user_context){
    view = view;//unused
    msg = msg;//unused
    event_args = event_args;//unused
    user_context = user_context;//unused
}

static void __set_view_visibility(View *view, bool is_visible){
    SDL_assert(view != NULL);

    if(view->mgr->root_view == view){
        SDL_ShowWindow(view->mgr->window);
    }
    else if(view->is_visible != is_visible){
        view->is_visible = is_visible;
        glv_redraw_window(view->mgr);
    }
}

static void __set_is_mouse_over(View *view, bool is_mouse_over){
    if(view->is_mouse_over == is_mouse_over) return;

    view->is_mouse_over = is_mouse_over;
    if(is_mouse_over){
        glv_push_event(view, VM_MOUSE_HOVER, NULL, 0);
    }
    else{
        glv_push_event(view, VM_MOUSE_LEAVE, NULL, 0);
    }
}

static void __unmap_child(View *child){
    SDL_assert(child != NULL);
    if(child->mgr->root_view == child || child->parent == NULL) return;

    View *parent = child->parent;

    View **src = parent->childs;
    View **dst = src;
    View **end = src + parent->childs_cnt;
    while (src != end){
        if(*src == child){
            src++;
        }
        else{
            *dst++ = *src++;
        }
    }
    parent->childs_cnt -= dst - src;
    parent->childs = realloc(parent->childs, parent->childs_cnt * sizeof(View*));

    if(dst - src > 1){
        glv_log_err(child->mgr, "parent contains two childs with same pointer");
    }
}

static void __enum_delete_childs(View *child, void *unused){
    unused = unused;//unused
    
    glv_enum_childs(child, __enum_delete_childs, unused);
    
    glv_delete(child);
}

static void __enum_call_mouse_button_event(View *child, void *args_ptr){
    if(child->is_visible == false) return;
    _MBEvEnumArgs *args = args_ptr;
    SDL_Point curr;

    args->ev.x -= child->x;
    args->ev.y -= child->y;

    curr.x = args->ev.x;
    curr.y = args->ev.y;

    if(curr.x < 0 || curr.y < 0
    || curr.x > (int)child->w || curr.y > (int)child->h){
        if(child->is_focused){
            child->is_focused = false;
            glv_push_event(child, VM_UNFOCUS, NULL, 0);
        }
        return;
    }

    glv_set_secondary_focus(child);
    glv_call_event(child, args->message, &args->ev, NULL);
    glv_call_manage(child, args->message, &args->ev);
    glv_enum_visible_childs(child, __enum_call_mouse_button_event, &args);

    args->ev.x = curr.x;
    args->ev.y = curr.y;
}

static void __enum_call_mouse_move_event(View *child, void *args_ptr){
    GlvMouseMove *args = args_ptr;
    SDL_Point curr;

    args->x -= child->x;
    args->y -= child->y;

    curr.x = args->x;
    curr.y = args->y;

    if(curr.x < 0 || curr.y < 0
    || curr.x > (int)child->w || curr.y > (int)child->h){
        __set_is_mouse_over(child, false);
        return;
    }

    __set_is_mouse_over(child, true);
    glv_call_event(child, VM_MOUSE_MOVE, args, NULL);
    glv_call_manage(child, VM_MOUSE_MOVE, args);
    glv_enum_visible_childs(child, __enum_call_mouse_move_event, args);
    
    args->x = curr.x;
    args->y = curr.y;
}

static void __enum_set_secondary_focus(View *view, void *unused){
    unused = unused;//unused
    glv_set_secondary_focus(view);
}

static void __enum_call_key_event(View *view, void *args_ptr){
    _KeyEvEnumArgs *args = args_ptr;

    glv_call_event(view, args->message, &args->ev, NULL);
    glv_call_manage(view, args->message, &args->ev);

    glv_enum_focused_childs(view, __enum_call_key_event, args);
}

static void __enum_call_textinput_event(View *view, void *args_ptr){
    glv_call_event(view, VM_TEXT, args_ptr, NULL);
    glv_call_manage(view, VM_TEXT, args_ptr);
    glv_enum_focused_childs(view, __enum_call_textinput_event, args_ptr);
}

static void __enum_call_textediting_event(View *view, void *args_ptr){
    glv_call_event(view, VM_TEXT_EDITING, args_ptr, NULL);
    glv_call_manage(view, VM_TEXT_EDITING, args_ptr);
    glv_enum_childs(view, __enum_call_textediting_event, args_ptr);
}

static void __enum_call_sdl_redirect_event(View *view, void *args_ptr){
    glv_call_event(view, VM_SDL_REDIRECT, args_ptr, NULL);
    glv_call_manage(view, VM_SDL_REDIRECT, args_ptr);
    glv_enum_visible_childs(view, __enum_call_sdl_redirect_event, args_ptr);
}

static void __enum_call_mouse_wheel(View *view, void *args_ptr){
    glv_call_event(view, VM_MOUSE_WHEEL, args_ptr, NULL);
    glv_call_manage(view, VM_MOUSE_WHEEL, args_ptr);
    glv_enum_hovered_childs(view, __enum_call_mouse_wheel, args_ptr);
}

static void __enum_draw_views(View *view, void *unused){
    
    if(view->is_redraw_queue){
        view->is_redraw_queue = false;
        
        glBindFramebuffer(GL_FRAMEBUFFER, view->framebuffer);
        glv_call_event(view, VM_DRAW, NULL, NULL);
        glv_call_manage(view, VM_DRAW, NULL);
    }
    glv_enum_childs(view, __enum_draw_views, unused);
}

static void __enum_point_to_parent(View *view, void *args){
    SDL_Point *p = args;

    p->x += view->x;
    p->y += view->y;
}

static void __enum_point_to_child(View *view, void *args){
    SDL_Point *p = args;
    p->x -= view->x;
    p->y -= view->y;
}

static void __handle_mouse_button(View *root, ViewMsg msg, const SDL_MouseButtonEvent *ev){
    _MBEvEnumArgs args;
    args.ev = (struct GlvEventMouseButton){
        .button = ev->button,
        .clicks = ev->clicks,
        .x = ev->x,
        .y = ev->y,
        .which = ev->which,
    };
    args.message = msg;
    
    if(root->is_visible){
        glv_set_secondary_focus(root);
        glv_call_event(root, msg, &args.ev, NULL);
        glv_call_manage(root, msg, &args.ev);
        glv_enum_visible_childs(root, __enum_call_mouse_button_event, &args);
    }

    args.message = msg;
}

static void __handle_mouse_move(View *root, const SDL_MouseMotionEvent *ev){
    GlvMouseMove args = {
        .x = ev->x,
        .y = ev->y,
        .which = ev->which
    };

    if(root->is_visible){
        __set_is_mouse_over(root, true);
        glv_call_event(root, VM_MOUSE_MOVE, &args, NULL);
        glv_call_manage(root, VM_MOUSE_MOVE, &args);
        glv_enum_visible_childs(root, __enum_call_mouse_move_event, &args);

    }
}

static void __handle_key(View *root, ViewMsg msg, const SDL_KeyboardEvent *ev){
    _KeyEvEnumArgs _ev = {
        .ev = {
            .sym = ev->keysym,
            .repeat = ev->repeat,
        },
        .message = msg,
    };

    glv_call_event(root, msg, &_ev.ev, NULL);
    glv_call_manage(root, msg, &_ev.ev);

    glv_enum_focused_childs(root, __enum_call_key_event, &_ev);
}

static void *__create_singleton_data_ifnexists(GlvMgr *mgr, ViewProc proc){
    SingletonData **curr = mgr->view_singleton_data;
    SingletonData **end = curr + mgr->view_singleton_data_cnt;

    while (curr != end){
        if(curr[0]->proc == proc){
            return (void*)&curr[0][1];
        }
        curr++;
    }

    Uint32 size = __get_singleton_data_size(proc);

    mgr->view_singleton_data_cnt++;
    mgr->view_singleton_data = realloc(
        mgr->view_singleton_data,
        mgr->view_singleton_data_cnt * sizeof(SingletonData*));
    
    SingletonData *data = malloc(sizeof(SingletonData) + size);
    memset(data, 0, sizeof(SingletonData) + size);
    data->proc = proc;

    mgr->view_singleton_data[mgr->view_singleton_data_cnt - 1] = data;
    
    return (void*)&data[1];
}

static void __delete_singletons(GlvMgr *mgr){
    SingletonData **curr = mgr->view_singleton_data;
    SingletonData **end = curr + mgr->view_singleton_data_cnt;

    while (curr != end){
        curr[0]->proc(NULL, VM_SINGLETON_DATA_DELETE, &curr[0][1], NULL);
        free(curr[0]);
        curr++;
    }
    free(mgr->view_singleton_data);
    mgr->view_singleton_data = NULL;
}

static Uint32 __get_singleton_data_size(ViewProc proc){
    Uint32 result = 0;
    proc(NULL, VM_GET_SINGLETON_DATA_SIZE, NULL, &result);
    return result;
}

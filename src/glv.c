#include "_glv.h"

static bool init_sdl(GlvMgr *mgr);
static bool init_window(GlvMgr *mgr);
static bool init_gl_ctx(GlvMgr *mgr);
static void create_mgr(GlvMgr *mgr);
static int delete_mgr(GlvMgr *mgr);
static void handle_events(GlvMgr *mgr, const SDL_Event *ev);
static void handle_winevents(GlvMgr *mgr, const SDL_WindowEvent *ev);
static void apply_events(GlvMgr *mgr);
static void draw_views_recursive(View *view);
static void handle_default_doc(ViewMsg msg, GlvMsgDocs *docs);
static void unfocus_all_excepting(View *view, View *exception);
static bool handle_private_message(View *view, ViewMsg message, const void *in);

void __create_root_view(GlvMgr *mgr, ViewProc view_proc, ViewManage manage_proc, void *user_context);
static void __draw_views_recursive(View *view, SDL_Rect parent);
static void __init_texture_1x1(View *view);
static void __init_framebuffer(View *view);
static void __manage_default(View *view, ViewMsg msg, const void *event_args, void *user_context);
static void __init_draw_texture_ifninit(GlvMgr *mgr);
static void __init_draw_texture_program(GlvMgr *mgr);
static void __init_draw_texture_vbo(GlvMgr *mgr);
static void __init_draw_texture_coord_bo(GlvMgr *mgr);
static void __init_draw_texture_var_locations(GlvMgr *mgr);
static void __free_draw_texture(GlvMgr *mgr);
static void __set_view_visibility(View *view, bool is_visible);

static void __enum_delete_childs(View *child, void *unused);
static void __enum_call_mouse_button_event(View *child, void *args_ptr);
static void __enum_call_mouse_move_event(View *child, void *args_ptr);
static void __enum_set_secondary_focus(View *view, void *unused);
static void __enum_call_key_event(View *view, void *args_ptr);

static void __handle_mouse_button(View *root, ViewMsg msg, const SDL_MouseButtonEvent *ev);
static void __handle_mouse_move(View *root, const SDL_MouseMotionEvent *ev);
static void __handle_key(View *root, ViewMsg msg, const SDL_KeyboardEvent *ev);

void __create_root_view(GlvMgr *mgr, ViewProc view_proc, ViewManage manage_proc, void *user_context){
    View *result = malloc(sizeof(View));
    SDL_zerop(result);
    mgr->root_view = result;

    result->mgr = mgr;
    result->view_proc = view_proc;
    result->user_context = user_context;
    result->is_visible = true;

    if(manage_proc == NULL) result->view_manage = __manage_default;
    else result->view_manage = manage_proc;
    
    __init_texture_1x1(result);
    __init_framebuffer(result);

    glv_call_event(result, VM_CREATE, NULL, NULL);
    glv_call_manage(result, VM_CREATE, NULL);
}

void glv_enum_childs(View *view, void(*enum_proc)(View *childs, void *data), void *data){
    View **curr = view->childs;
    View **end = curr + view->childs_cnt;

    while (curr != end){
        enum_proc(*curr, data);
        curr++;
    }
}

void glv_enum_visible_childs(View *view, void(*enum_proc)(View *childs, void *data), void *data){
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
    View **curr = view->childs;
    View **end = curr + view->childs_cnt;

    while (curr != end){
        if(curr[0]->is_focused){
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

    if(manage_proc == NULL) result->view_manage = __manage_default;
    else result->view_manage = manage_proc;

    __init_texture_1x1(result);
    __init_framebuffer(result);

    parent->childs_cnt++;
    parent->childs = realloc(parent->childs, parent->childs_cnt * sizeof(View*));
    parent->childs[parent->childs_cnt - 1] = result;

    glv_push_event(result, VM_CREATE, NULL, 0);
    glv_push_event(parent, VM_CHILD_CREATE, &result, sizeof(View*));

    return result;
}

void glv_delete(View *view){
    if(view == NULL) return;
    if(view == view->mgr->root_view){
        view->mgr->root_view = NULL;
        view->mgr->is_running = false;
    }
    else{
        glv_push_event(view->parent, VM_CHILD_DELETE, &view, sizeof(View*));
    }

    glv_enum_childs(view, __enum_delete_childs, NULL);

    glv_push_event(view, VM_VIEW_FREE__, NULL, 0);
}

GlvMgr *glv_get_mgr(View *view){
    SDL_assert(view != NULL);
    return view->mgr;
}

void glv_manage_default(View *view, ViewMsg msg, const void *event_args, void *user_context){
    view = view;//unused
    event_args = event_args;//unused
    user_context = user_context;//unused
    msg = msg;//unused
}

void glv_proc_default(View *view, ViewMsg msg, const void *in, void *out){
    in = in;//unused
    out = out;//unused
    view = view;//unused
    switch (msg){
    case VM_GET_DOCS:
        handle_default_doc(*(ViewMsg*)in, out);
        break;
    }
}

int glv_run(ViewProc root_view_proc, ViewManage root_view_manage, void *root_user_data, void (*init_spa)(View *root_view)){
    GlvMgr mgr;
    create_mgr(&mgr);
    mgr.is_running = true;

    if(!init_sdl(&mgr)
    || !init_window(&mgr)
    || !init_gl_ctx(&mgr)){
        glv_log_err(&mgr, NULL);
        return EXIT_FAILURE;
    }

    SDL_DisplayMode window_display_mode;
    SDL_GetWindowDisplayMode(mgr.window, &window_display_mode);
    glv_set_minimal_frametime(&mgr, 1000 / window_display_mode.refresh_rate);

    __init_draw_texture_ifninit(&mgr);

    __create_root_view(&mgr, root_view_proc, root_view_manage, root_user_data);


    init_spa(mgr.root_view);

    SDL_ShowWindow(mgr.window);
    SDL_Event ev;
    while (mgr.is_running){
        while(SDL_PollEvent(&ev)){
            handle_events(&mgr, &ev);
        }

        apply_events(&mgr);
        SDL_Delay(mgr.min_frametime_ms);
    }

    glv_delete(mgr.root_view);
    return delete_mgr(&mgr);
}

void glv_push_event(View *view, ViewMsg message, const void *args, uint32_t args_size){
    SDL_assert(view != NULL);
    SDL_assert((args_size != 0 && args == NULL) == false);

    SDL_Event ev;
    GlvSdlEvent *glv_ev = (GlvSdlEvent*)&ev.user;
    glv_ev->type = SDL_USEREVENT;
    glv_ev->message = message;
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

void glv_call_event(View *view, ViewMsg message, const void *in, void *out){
    SDL_assert(view != NULL);
    view->view_proc(view, message, in, out);
}

void glv_call_manage(View *view, ViewMsg message, const void *event_args){
    SDL_assert(view != NULL);
    view->view_manage(view, message, event_args, view->user_context);
}

void glv_bind_view_framebuffer(View *view){
    glBindFramebuffer(GL_FRAMEBUFFER, view->framebuffer);
}

GLuint glv_get_texture(View *view){
    return view->texture;
}

GlvMsgDocs glv_get_docs(View *view, ViewMsg message){
    GlvMsgDocs result;
    SDL_zero(result);
    result.msg = VM_NULL;

    if(message == VM_GET_DOCS){
        result.name = SDL_STRINGIFY_ARG(VM_GET_DOCS);
        result.input_description = "const ViewMsg *message";
        result.output_description = "GlvMsgDocs *message_docs";
        result.description = "describes passed message";
    }
    else{
        view->view_proc(view, VM_GET_DOCS, &message, &result);
        if(result.msg == VM_NULL){
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
    docs->msg = message;
    docs->name = name;
    docs->input_description = input_description;
    docs->output_description = output_description;
    docs->description = general_description;
}

void glv_set_minimal_frametime(GlvMgr *mgr, Uint32 ms_min_frametime){
    mgr->min_frametime_ms = ms_min_frametime;
}

void glv_set_focus(View *view){
    SDL_assert(view != NULL);

    unfocus_all_excepting(view, view);
    glv_set_secondary_focus(view);
    glv_enum_parents(view, __enum_set_secondary_focus, NULL);
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
    unfocus_all_excepting(view, NULL);
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
    if(view == view->mgr->root_view){
        SDL_SetWindowPosition(view->mgr->window, x, y);
    }
    else{
        view->x = x;
        view->y = y;

        SDL_Point new_pos;
        glv_push_event(view, VM_MOVE, &new_pos, sizeof(new_pos));
        glv_push_event(view->parent, VM_CHILD_MOVE, &view, sizeof(View*));
        glv_redraw_window(view->mgr);
    }
}

void glv_set_size(View *view, unsigned int width, unsigned int height){
    if(view == view->mgr->root_view){
        SDL_SetWindowSize(view->mgr->window, width, height);
    }
    else{
        view->w = width;
        view->h = height;
        SDL_Point new_size;
        glv_push_event(view, VM_RESIZE, &new_size, sizeof(new_size));
        glv_push_event(view->parent, VM_CHILD_RESIZE, &view, sizeof(View*));
        glv_redraw_window(view->mgr);
    }
}

void glv_draw(View *view){
    view->is_drawable = true;
    glv_push_event(view, VM_DRAW, NULL, 0);
    glv_redraw_window(view->mgr);
}

void glv_show(View *view){
    __set_view_visibility(view, true);
    glv_push_event(view, VM_SHOW, NULL, 0);
}

void glv_hide(View *view){
    __set_view_visibility(view, false);
    glv_push_event(view, VM_HIDE, NULL, 0);
    unfocus_all_excepting(view, NULL);
}

static bool init_sdl(GlvMgr *mgr){
    if(SDL_Init(SDL_INIT_VIDEO) < 0){
        glv_log_err(mgr, SDL_GetError());
        return false;
    }
    else{
        return true;
    }
}

static bool init_window(GlvMgr *mgr){
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

static bool init_gl_ctx(GlvMgr *mgr){
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

static void create_mgr(GlvMgr *mgr){
    SDL_zerop(mgr);
    mgr->logger_proc = log_printf;
    mgr->min_frametime_ms = 10;
    mgr->last_frame_drawed_time_ms = SDL_GetTicks();
}

static int delete_mgr(GlvMgr *mgr){
    __free_draw_texture(mgr);
    SDL_DestroyWindow(mgr->window);
    SDL_GL_DeleteContext(mgr);
    return mgr->return_code;
}

static void handle_events(GlvMgr *mgr, const SDL_Event *ev){
    switch (ev->type){
        case SDL_WINDOWEVENT:
            handle_winevents(mgr, &ev->window);
        break;
        case SDL_QUIT:{
            mgr->is_running = false;
        }break;
        case SDL_USEREVENT:{
            const GlvSdlEvent *glv_ev = (const GlvSdlEvent*)&ev->user;
            if(!handle_private_message(glv_ev->view, glv_ev->message, glv_ev->data)){
                glv_call_event(glv_ev->view, glv_ev->message, glv_ev->data, NULL);
                glv_call_manage(glv_ev->view, glv_ev->message, glv_ev->data);
            }
            free(glv_ev->data);
        } break;
        case SDL_MOUSEBUTTONDOWN:
            if(ev->button.windowID != mgr->wind_id) break;;
            __handle_mouse_button(mgr->root_view, VM_MOUSE_DOWN, &ev->button);
        break;
        case SDL_MOUSEBUTTONUP:
            if(ev->button.windowID != mgr->wind_id) break;;
            __handle_mouse_button(mgr->root_view, VM_MOUSE_UP, &ev->button);
        break;
        case SDL_MOUSEMOTION:{
            if(ev->button.windowID != mgr->wind_id) break;;
            __handle_mouse_move(mgr->root_view, &ev->motion);
        }break;
        case SDL_KEYDOWN:{
            if(ev->button.windowID != mgr->wind_id) break;;
            __handle_key(mgr->root_view, VM_KEY_DOWN, &ev->key);
        }break;
        case SDL_KEYUP:{
            if(ev->button.windowID != mgr->wind_id) break;;
            __handle_key(mgr->root_view, VM_KEY_UP, &ev->key);
        }break;
    }
}

static void handle_winevents(GlvMgr *mgr, const SDL_WindowEvent *ev){
    if(ev->windowID != mgr->wind_id) return;
    if(mgr->root_view == NULL){
        glv_log_err(mgr, "root view is NULL");
        return;
    }

    switch (ev->event){
    case SDL_WINDOWEVENT_EXPOSED:
        mgr->required_redraw = true;
        break;
    case SDL_WINDOWEVENT_RESIZED:{
        SDL_Point new_size = {
            .x = mgr->root_view->w = ev->data1,
            .y = mgr->root_view->h = ev->data2,
        };

        glv_push_event(mgr->root_view, VM_RESIZE, &new_size, sizeof(new_size));
    }break;
    case SDL_WINDOWEVENT_SHOWN:{
        SDL_Point new_pos;
        SDL_Point new_size;

        mgr->root_view->is_visible = true;
        glv_push_event(mgr->root_view, VM_SHOW, NULL, 0);

        SDL_GetWindowSize(mgr->window, &new_size.x, &new_size.y);
        SDL_GetWindowPosition(mgr->window, &new_pos.x, &new_pos.y);

        glv_push_event(mgr->root_view, VM_MOVE, &new_size, sizeof(new_pos));
        glv_push_event(mgr->root_view, VM_RESIZE, &new_size, sizeof(new_size));
    }break;
    case SDL_WINDOWEVENT_HIDDEN:{
        mgr->root_view->is_visible = false;
        glv_push_event(mgr->root_view, VM_HIDE, NULL, 0);
    }break;
    case SDL_WINDOWEVENT_FOCUS_GAINED:{
        mgr->root_view->is_focused = true;
        glv_push_event(mgr->root_view, VM_FOCUS, NULL, 0);
    }break;
    case SDL_WINDOWEVENT_FOCUS_LOST:{
        mgr->root_view->is_focused = false;
        glv_push_event(mgr->root_view, VM_UNFOCUS, NULL, 0);
    }break;
    }
}

static void apply_events(GlvMgr *mgr){
    if(should_redraw(mgr)){
        if(mgr->root_view == NULL){
            glv_log_err(mgr, "redrawing NULL View");
            return;
        }
    
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        SDL_Point new_size;
        SDL_GetWindowSize(mgr->window, &new_size.x, &new_size.y);
        glViewport(0, 0, new_size.x, new_size.y);

        draw_views_recursive(mgr->root_view);
        SDL_GL_SwapWindow(mgr->window);
    }
}

static void draw_views_recursive(View *view){
    SDL_Rect parent = {
        .x = 0,
        .y = 0
    };
    SDL_GetWindowSize(view->mgr->window, &parent.w, &parent.h);
    
    __draw_views_recursive(view, parent);
}

#define __DOC_CASE(MESSAGE, INPUT, OUTPUT, DESCRIPTION)\
    case MESSAGE:\
        glv_write_docs(\
        docs, MESSAGE,\
        SDL_STRINGIFY_ARG(MESSAGE),\
        INPUT, OUTPUT, DESCRIPTION); break;

static void handle_default_doc(ViewMsg msg, GlvMsgDocs *docs){
    switch (msg){
        __DOC_CASE(VM_NULL, "NULL", "NULL", "doesnt handles");
        __DOC_CASE(VM_CREATE, "NULL", "NULL", "calls once on create");
        __DOC_CASE(VM_DELETE, "NULL", "NULL", "calls once on delete");
        __DOC_CASE(VM_RESIZE, "const SDL_Point *new_size", "NULL", "calls when view is resized");
        __DOC_CASE(VM_MOVE, "const SDL_Point *new_pos", "NULL", "calls when view is moved");
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
        free(view->view_data);
        free(view);
        return true;
    default: return false;
    }
}

static void __enum_set_secondary_focus(View *view, void *unused){
    unused = unused;//unused
    glv_set_secondary_focus(view);
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


static void __init_texture_1x1(View *view){
    glGenTextures(1, &view->texture);
    glBindTexture(GL_TEXTURE_2D, view->texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glBindTexture(GL_TEXTURE_2D, 0);
}

static void __init_framebuffer(View *view){
    glGenFramebuffers(1, &view->framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, view->framebuffer);
    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, view->texture, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void __manage_default(View *view, ViewMsg msg, const void *event_args, void *user_context){
    view = view;//unused
    msg = msg;//unused
    event_args = event_args;//unused
    user_context = user_context;//unused
}



static void __init_draw_texture_ifninit(GlvMgr *mgr){
    if(mgr->draw_texture_program.prog == 0){
        __init_draw_texture_program(mgr);
        __init_draw_texture_vbo(mgr);
        __init_draw_texture_coord_bo(mgr);
        __init_draw_texture_var_locations(mgr);
    }
}

static void __init_draw_texture_program(GlvMgr *mgr){
    GLuint f = glCreateShader(GL_FRAGMENT_SHADER);
    GLuint v = glCreateShader(GL_VERTEX_SHADER);

    glShaderSource(f, 1, (const GLchar**)&draw_texture_frag, NULL);
    glShaderSource(v, 1, (const GLchar**)&draw_texture_vert, NULL);

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
        glv_log_err(mgr, "cannot compile mgr.draw_texture_program");

        char buffer[128];
        glGetProgramInfoLog(prog, sizeof(buffer), NULL, buffer);
        glv_log_err(mgr, buffer);
        glDeleteProgram(prog);

        mgr->is_running = false;
    }
    else{
        mgr->draw_texture_program.prog = prog;
    }
}

static void __init_draw_texture_vbo(GlvMgr *mgr){
    glGenBuffers(1, &mgr->draw_texture_program.vbo);
    GLuint vbo = mgr->draw_texture_program.vbo;

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    float buf_data[] = {
        -1,1, 1,1, -1,-1,
        1,-1, 1,1, -1,-1
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(buf_data), buf_data, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

static void __init_draw_texture_coord_bo(GlvMgr *mgr){
    glGenBuffers(1, &mgr->draw_texture_program.coords_bo);
    GLuint coords_bo = mgr->draw_texture_program.coords_bo;

    glBindBuffer(GL_ARRAY_BUFFER, coords_bo);
    float buf_data[] = {
        0,1, 1,1, 0,0,
        1,0, 1,1, 0,0
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(buf_data), buf_data, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}


static void __init_draw_texture_var_locations(GlvMgr *mgr){
    DrawTextureProgram *p = &mgr->draw_texture_program;

    p->vertex_p = glGetAttribLocation(p->prog, "vertex_p");
    p->tex_coords_p = glGetAttribLocation(p->prog, "tex_coords");

    p->tex_p = glGetUniformLocation(p->prog, "tex");
    p->mvp_p = glGetUniformLocation(p->prog, "mvp");
    p->tex_mvp_p = glGetUniformLocation(p->prog, "tex_mvp");
}

static void __free_draw_texture(GlvMgr *mgr){
    DrawTextureProgram *p = &mgr->draw_texture_program;
    
    if(p->prog) glDeleteProgram(p->prog);
    if(p->vbo) glDeleteBuffers(1, &p->vbo);
    if(p->tex_coords_p) glDeleteBuffers(1, &p->tex_coords_p);

    p->prog = 0;
    p->vbo = 0;
    p->tex_coords_p = 0;
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

static void __enum_delete_childs(View *child, void *unused){
    unused = unused;//unused
    glv_delete(child);

    glv_enum_childs(child, __enum_delete_childs, unused);
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

    if(curr.x < 0 || curr.y < 0) return;
    if(curr.x > (int)child->w || curr.y > (int)child->h) return;

    glv_call_event(child, VM_MOUSE_MOVE, args, NULL);
    glv_call_manage(child, VM_MOUSE_MOVE, args);
    glv_enum_visible_childs(child, __enum_call_mouse_button_event, args);
    
    args->x = curr.x;
    args->y = curr.y;
}

static void __enum_call_key_event(View *view, void *args_ptr){
    _KeyEvEnumArgs *args = args_ptr;

    glv_call_event(view, args->message, &args->ev, NULL);
    glv_call_manage(view, args->message, &args->ev);

    glv_enum_focused_childs(view, __enum_call_key_event, args);
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

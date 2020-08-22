#if !defined(FCODER_DEFAULT_BINDINGS_CPP)
#define FCODER_DEFAULT_BINDINGS_CPP

#include "4coder_default_include.cpp"

// NOTE(allen): Users can declare their own managed IDs here.
CUSTOM_ID(command_map, mapid_delete);
Command_Map_ID PreviousMapID;

#if !defined(META_PASS)
#include "generated/managed_id_metadata.cpp"
#endif

typedef i8  I8;
typedef i16 I16;
typedef i32 I32;
typedef i64 I64;

typedef u8  U8;
typedef u16 U16;
typedef u32 U32;
typedef u64 U64;

typedef f32 F32;
typedef f64 F64;

#define U8_MAX  (U8) 0xFF
#define U16_MAX (U16)0xFFFF
#define U32_MAX (U32)0xFFFFFFFF
#define U64_MAX (U64)0xFFFFFFFFFFFFFFFF

#define I8_MAX  (I8) (U8_MAX  >> 1)
#define I16_MAX (I16)(U16_MAX >> 1)
#define I32_MAX (I32)(U32_MAX >> 1)
#define I64_MAX (I64)(U64_MAX >> 1)

#define I8_MIN  (I8) U8_MAX
#define I16_MIN (I16)U16_MAX
#define I32_MIN (I32)U32_MAX
#define I64_MIN (I64)U64_MAX

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))

#define ASSERT(EX) ((EX) ? 0 : *(volatile int*)0)
#define NOT_IMPLEMENTED ASSERT(!"NOT_IMPLEMENTED")
#define INVALID_CODE_PATH ASSERT(!"INVALID_CODE_PATH")
#define INVALID_DEFAULT_CASE ASSERT(!"INVALID_DEFAULT_CASE")

#include <stdlib.h>

#include "soimn_string.h"
#include "soimn_memory.h"

ARGB_Color SoimnFunctionColor    = 0xFF99513D;
ARGB_Color SoimnTypeColor        = 0xFFCD950C;
ARGB_Color SoimnMacroColor       = 0xFF41776E;
ARGB_Color SoimnEnumColor        = 0xFF6E7741;
ARGB_Color SoimnNoteColor        = 0xFF00A000;
ARGB_Color SoimnTodoColor        = 0xFFA00000;
ARGB_Color SoimnImportantColor   = 0xFFBBBB22;
ARGB_Color SoimnHackColor        = 0xFFAA00AA;
ARGB_Color SoimnTempColor        = 0xFFA00000;
ARGB_Color SoimnHighCommentColor = 0xFFBBBBBB;
ARGB_Color SoimnErrCommentColor  = 0xFFEE5830;

struct Code_Note
{
    Code_Index_Note_Kind note_kind;
    Range_i64 pos;
    String_Const_u8 text;
    struct Code_Index_File* file;
    char peek[2];
};

struct Code_Note_Array
{
    Code_Note_Array* next;
    U32 size;
    U32 capacity;
};

struct Buffer_Info
{
    Memory_Arena arena;
    
    Code_Note_Array* note_arrays[27 + 27 + 1];
};

Buffer_Info* BufferInfos = 0;
U32 BufferInfoCount      = 0;
U32 BufferInfoCapacity   = 0;

bool NoteArraysInitialized               = false;
Code_Note_Array* NoteArrays[27 + 27 + 1] = {};

CUSTOM_COMMAND_SIG(SoimnTryExit)
{
    User_Input input = get_current_input(app);
    if (match_core_code(&input, CoreCode_TryExit))
    {
        bool has_unsaved_changes = false;
        
        for (Buffer_ID buffer = get_buffer_next(app, 0, Access_Always);
             buffer != 0;
             buffer = get_buffer_next(app, buffer, Access_Always))
        {
            Dirty_State dirty = buffer_get_dirty_state(app, buffer);
            
            if (HasFlag(dirty, DirtyState_UnsavedChanges))
            {
                has_unsaved_changes = true;
                break;
            }
        }
        
        View_ID view = get_active_view(app, Access_Always);
        
        if (!has_unsaved_changes || do_4coder_close_user_check(app, view))
        {
            hard_exit(app);
        }
    }
}

void
SoimnMoveToPanel(Application_Links* app, U8 direction)
{
    View_ID view_id = get_active_view(app, Access_Always);
    
    Panel_ID original_panel = view_get_panel(app, view_id);
    Rect_f32 original_rect  = view_get_screen_rect(app, view_id);
    
    
    Side preferred_side         = (direction == 0 || direction == 1 ? Side_Min : Side_Max);
    bool should_move_horizontal = (direction == 1 || direction == 3);
    
    Panel_ID move_top_node = 0;
    
    Panel_ID current_panel = original_panel;
    while (move_top_node == 0)
    {
        Panel_ID parent = panel_get_parent(app, current_panel);
        if (!parent) break;
        
        Panel_ID min_panel = panel_get_child(app, parent, Side_Min);
        Panel_ID max_panel = panel_get_child(app, parent, Side_Max);
        
        if (current_panel == min_panel && preferred_side == Side_Min ||
            current_panel == max_panel && preferred_side == Side_Max)
        {
            current_panel = parent;
            continue;
        }
        
        else
        {
            Panel_ID min_corner = min_panel;
            while (!panel_is_leaf(app, min_corner)) min_corner = panel_get_child(app, min_corner, Side_Min);
            
            Panel_ID max_corner = max_panel;
            while (!panel_is_leaf(app, max_corner)) max_corner = panel_get_child(app, max_corner, Side_Min);
            
            Rect_f32 min_origin_rect = view_get_screen_rect(app, panel_get_view(app, min_corner, Access_Always));
            Rect_f32 max_origin_rect = view_get_screen_rect(app, panel_get_view(app, max_corner, Access_Always));
            
            bool split_is_vertical   = (min_origin_rect.y0 == max_origin_rect.y0);
            bool split_is_horizontal = (min_origin_rect.x0 == max_origin_rect.x0);
            
            if (should_move_horizontal && split_is_vertical || !should_move_horizontal && split_is_horizontal)
            {
                move_top_node = (preferred_side == Side_Min ? min_panel : max_panel);
                break;
            }
            
            else
            {
                current_panel = parent;
                continue;
            }
        }
    }
    
    if (move_top_node != 0)
    {
        while (!panel_is_leaf(app, move_top_node))
        {
            Panel_ID min_panel = panel_get_child(app, move_top_node, Side_Min);
            Panel_ID max_panel = panel_get_child(app, move_top_node, Side_Max);
            
            Panel_ID min_corner = min_panel;
            while (!panel_is_leaf(app, min_corner)) min_corner = panel_get_child(app, min_corner, Side_Min);
            
            Panel_ID max_corner = max_panel;
            while (!panel_is_leaf(app, max_corner)) max_corner = panel_get_child(app, max_corner, Side_Min);
            
            Rect_f32 min_origin_rect = view_get_screen_rect(app, panel_get_view(app, min_corner, Access_Always));
            Rect_f32 max_origin_rect = view_get_screen_rect(app, panel_get_view(app, max_corner, Access_Always));
            
            bool split_is_vertical   = (min_origin_rect.y0 == max_origin_rect.y0);
            bool split_is_horizontal = (min_origin_rect.x0 == max_origin_rect.x0);
            
            if (should_move_horizontal && split_is_vertical || !should_move_horizontal && split_is_horizontal)
            {
                if (preferred_side == Side_Min) move_top_node = max_panel;
                else                            move_top_node = min_panel;
            }
            
            else
            {
                F32 dist_from_min = 0;
                F32 dist_from_max = 0;
                
                if (split_is_vertical)
                {
                    dist_from_min = original_rect.x0 - min_origin_rect.x0;
                    dist_from_max = original_rect.x0 - max_origin_rect.x0;
                }
                
                else
                {
                    dist_from_min = original_rect.y0 - min_origin_rect.y0;
                    dist_from_max = original_rect.y0 - max_origin_rect.y0;
                }
                
                if (dist_from_max < 0 || dist_from_min < dist_from_max) move_top_node = min_panel;
                else                                                    move_top_node = max_panel;
            }
        }
        
        View_ID target_view = panel_get_view(app, move_top_node, Access_Always);
        view_set_active(app, target_view);
    }
}

CUSTOM_COMMAND_SIG(SoimnMoveToPanelUp)
{
    SoimnMoveToPanel(app, 0);
}

CUSTOM_COMMAND_SIG(SoimnMoveToPanelLeft)
{
    SoimnMoveToPanel(app, 1);
}

CUSTOM_COMMAND_SIG(SoimnMoveToPanelDown)
{
    SoimnMoveToPanel(app, 2);
}

CUSTOM_COMMAND_SIG(SoimnMoveToPanelRight)
{
    SoimnMoveToPanel(app, 3);
}

CUSTOM_COMMAND_SIG(SoimnCopyLine)
{
    View_ID view = get_active_view(app, Access_ReadVisible);
    I64 pos      = view_get_cursor_pos(app, view);
    
    Buffer_Cursor cursor = view_compute_cursor(app, view, seek_pos(pos));
    Vec2_f32 p           = view_relative_xy_of_pos(app, view, cursor.line, pos);
    
    I64 start = view_pos_at_relative_xy(app, view, cursor.line, {0, p.y});
    I64 end   = view_pos_at_relative_xy(app, view, cursor.line, {max_f32, p.y});
    
    Buffer_ID buffer = view_get_buffer(app, view, Access_Always);
    clipboard_post_buffer_range(app, 0, buffer, Ii64(start, end));
}

CUSTOM_COMMAND_SIG(SoimnCutLine)
{
    SoimnCopyLine(app);
    delete_line(app);
}

CUSTOM_COMMAND_SIG(SoimnEnterDeleteMode)
{
    View_ID view     = get_this_ctx_view(app, Access_Always);
    Buffer_ID buffer = view_get_buffer(app, view, Access_Always);
    
    Managed_Scope scope        = buffer_get_managed_scope(app, buffer);
    Command_Map_ID *map_id_ptr = scope_attachment(app, scope, buffer_map_id, Command_Map_ID);
    
    PreviousMapID = *map_id_ptr;
    *map_id_ptr = mapid_delete;
}

CUSTOM_COMMAND_SIG(SoimnExitDeleteMode)
{
    View_ID view     = get_this_ctx_view(app, Access_Always);
    Buffer_ID buffer = view_get_buffer(app, view, Access_Always);
    
    Managed_Scope scope        = buffer_get_managed_scope(app, buffer);
    Command_Map_ID *map_id_ptr = scope_attachment(app, scope, buffer_map_id, Command_Map_ID);
    
    *map_id_ptr = PreviousMapID;
}

CUSTOM_COMMAND_SIG(SoimnDeleteLeft)
{
    View_ID view = get_active_view(app, Access_ReadVisible);
    I64 pos      = view_get_cursor_pos(app, view);
    
    Buffer_Cursor cursor = view_compute_cursor(app, view, seek_pos(pos));
    Vec2_f32 p           = view_relative_xy_of_pos(app, view, cursor.line, pos);
    I64 start            = view_pos_at_relative_xy(app, view, cursor.line, {0, p.y});
    
    Buffer_ID buffer = view_get_buffer(app, view, Access_Always);
    buffer_replace_range(app, buffer, Ii64(start, pos), string_u8_litexpr(""));
    
    SoimnExitDeleteMode(app);
}

CUSTOM_COMMAND_SIG(SoimnDeleteRight)
{
    View_ID view = get_active_view(app, Access_ReadVisible);
    I64 pos      = view_get_cursor_pos(app, view);
    
    Buffer_Cursor cursor = view_compute_cursor(app, view, seek_pos(pos));
    Vec2_f32 p           = view_relative_xy_of_pos(app, view, cursor.line, pos);
    I64 end              = view_pos_at_relative_xy(app, view, cursor.line, {max_f32, p.y});
    
    Buffer_ID buffer = view_get_buffer(app, view, Access_Always);
    buffer_replace_range(app, buffer, Ii64(pos, end), string_u8_litexpr(""));
    
    SoimnExitDeleteMode(app);
}

CUSTOM_COMMAND_SIG(SoimnDeleteRange)
{
    delete_range(app);
    SoimnExitDeleteMode(app);
}

void
SoimnBindKeys(Mapping* mapping)
{
    MappingScope();
    SelectMapping(mapping);
    
    /// GLOBAL BINDINGS
    
    SelectMap(mapid_global);
    BindCore(default_startup, CoreCode_Startup);
    BindCore(default_try_exit, CoreCode_TryExit);
    
    Bind(interactive_new,               KeyCode_N, KeyCode_Control);
    Bind(interactive_open_or_new,       KeyCode_O, KeyCode_Control);
    Bind(interactive_kill_buffer,       KeyCode_K, KeyCode_Control);
    Bind(interactive_switch_buffer,     KeyCode_I, KeyCode_Control);
    
    Bind(close_build_panel,             KeyCode_Comma, KeyCode_Control);
    
    Bind(command_lister,                KeyCode_X, KeyCode_Alt);
    Bind(project_command_lister,        KeyCode_X, KeyCode_Alt, KeyCode_Shift);
    
    Bind(project_fkey_command, KeyCode_F1);
    Bind(project_fkey_command, KeyCode_F2);
    Bind(project_fkey_command, KeyCode_F3);
    Bind(project_fkey_command, KeyCode_F4);
    Bind(project_fkey_command, KeyCode_F5);
    Bind(project_fkey_command, KeyCode_F6);
    Bind(project_fkey_command, KeyCode_F7);
    Bind(project_fkey_command, KeyCode_F8);
    Bind(project_fkey_command, KeyCode_F9);
    Bind(project_fkey_command, KeyCode_F10);
    Bind(project_fkey_command, KeyCode_F11);
    Bind(project_fkey_command, KeyCode_F12);
    Bind(project_fkey_command, KeyCode_F13);
    Bind(project_fkey_command, KeyCode_F14);
    Bind(project_fkey_command, KeyCode_F15);
    Bind(project_fkey_command, KeyCode_F16);
    
    Bind(list_all_functions_all_buffers_lister, KeyCode_I, KeyCode_Control, KeyCode_Shift);
    
    BindMouseWheel(mouse_wheel_scroll);
    BindMouseWheel(mouse_wheel_change_face_size, KeyCode_Control);
    
    /// FILE BINDINGS
    
    SelectMap(mapid_file);
    ParentMap(mapid_global);
    BindTextInput(write_text_input);
    
    BindMouse(click_set_cursor_and_mark, MouseCode_Left);
    BindMouseRelease(click_set_cursor, MouseCode_Left);
    BindCore(click_set_cursor_and_mark, CoreCode_ClickActivateView);
    BindMouseMove(click_set_cursor_if_lbutton);
    
    Bind(save,                        KeyCode_S, KeyCode_Control);
    Bind(redo,                        KeyCode_Y, KeyCode_Control);
    Bind(undo,                        KeyCode_Z, KeyCode_Control);
    
    Bind(move_up,                     KeyCode_Up);
    Bind(move_down,                   KeyCode_Down);
    Bind(move_left,                   KeyCode_Left);
    Bind(move_right,                  KeyCode_Right);
    
    Bind(move_left_token_boundary,    KeyCode_Left,  KeyCode_Control);
    Bind(move_right_token_boundary,   KeyCode_Right, KeyCode_Control);
    Bind(move_up_to_blank_line,       KeyCode_Up,    KeyCode_Control);
    Bind(move_down_to_blank_line,     KeyCode_Down,  KeyCode_Control);
    
    Bind(move_up_10,                  KeyCode_Up,    KeyCode_Shift);
    Bind(move_down_10,                KeyCode_Down,  KeyCode_Shift);
    Bind(seek_end_of_line,            KeyCode_Right, KeyCode_Shift);
    Bind(seek_beginning_of_line,      KeyCode_Left,  KeyCode_Shift);
    
    Bind(move_line_up,                KeyCode_Up,   KeyCode_Alt);
    Bind(move_line_down,              KeyCode_Down, KeyCode_Alt);
    
    Bind(page_up, KeyCode_PageUp);
    Bind(page_down, KeyCode_PageDown);
    
    Bind(SoimnMoveToPanelUp,          KeyCode_Up,    KeyCode_Alt, KeyCode_Shift);
    Bind(SoimnMoveToPanelDown,        KeyCode_Down,  KeyCode_Alt, KeyCode_Shift);
    Bind(SoimnMoveToPanelLeft,        KeyCode_Left,  KeyCode_Alt);
    Bind(SoimnMoveToPanelRight,       KeyCode_Right, KeyCode_Alt);
    
    Bind(search,                      KeyCode_F, KeyCode_Control);
    Bind(goto_line,                   KeyCode_G, KeyCode_Control);
    Bind(center_view,                 KeyCode_E, KeyCode_Control);
    Bind(left_adjust_view,            KeyCode_E, KeyCode_Control, KeyCode_Shift);
    
    Bind(set_mark,                    KeyCode_Space, KeyCode_Control);
    Bind(cursor_mark_swap,            KeyCode_M, KeyCode_Control);
    
    Bind(delete_char,                 KeyCode_Delete);
    Bind(backspace_char,              KeyCode_Backspace);
    Bind(copy,                        KeyCode_C,    KeyCode_Control);
    Bind(SoimnCopyLine,               KeyCode_C,    KeyCode_Control, KeyCode_Shift);
    Bind(paste,                       KeyCode_V,    KeyCode_Control);
    Bind(cut,                         KeyCode_X,    KeyCode_Control);
    Bind(SoimnCutLine,                KeyCode_X,    KeyCode_Control, KeyCode_Shift);
    Bind(query_replace,               KeyCode_Q,    KeyCode_Control);
    Bind(SoimnEnterDeleteMode,        KeyCode_D,    KeyCode_Control);
    Bind(delete_line,                 KeyCode_D,    KeyCode_Control, KeyCode_Shift);
    Bind(backspace_alpha_numeric_boundary, KeyCode_Backspace, KeyCode_Control);
    Bind(delete_alpha_numeric_boundary,    KeyCode_Delete,    KeyCode_Control);
    
    /// DELETE BINDINGS
    
    SelectMap(mapid_delete);
    ParentMap(mapid_global);
    Bind(SoimnDeleteLeft,             KeyCode_Left);
    Bind(SoimnDeleteRight,            KeyCode_Right);
    Bind(SoimnDeleteRange,            KeyCode_D);
    Bind(SoimnExitDeleteMode,         KeyCode_Escape);
    
    /// CODE BINDINGS
    
    SelectMap(mapid_code);
    ParentMap(mapid_file);
    BindTextInput(write_text_and_auto_indent);
    
    Bind(word_complete, KeyCode_Tab);
}

void
SoimnSetColors(Application_Links* app)
{
    if (global_theme_arena.base_allocator == 0)
    {
        global_theme_arena = make_arena_system();
    }
    
    Arena* arena = &global_theme_arena;
    
    default_color_table = make_color_table(app, arena);
    
    default_color_table.arrays[0]                              = make_colors(arena, 0xFF90B080);
    
    default_color_table.arrays[defcolor_bar]                   = make_colors(arena, 0xFFCACACA);
    default_color_table.arrays[defcolor_base]                  = make_colors(arena, 0xFF000000);
    default_color_table.arrays[defcolor_pop1]                  = make_colors(arena, 0xFF03CF0C);
    default_color_table.arrays[defcolor_pop2]                  = make_colors(arena, 0xFFFF0000);
    default_color_table.arrays[defcolor_back]                  = make_colors(arena, 0xFF161616);
    default_color_table.arrays[defcolor_margin]                = make_colors(arena, 0xFF262626);
    default_color_table.arrays[defcolor_margin_hover]          = make_colors(arena, 0xFF333333);
    default_color_table.arrays[defcolor_margin_active]         = make_colors(arena, 0xFF404040);
    default_color_table.arrays[defcolor_list_item]             = make_colors(arena, 0xFF262626);
    default_color_table.arrays[defcolor_list_item_hover]       = make_colors(arena, 0xFF333333);
    default_color_table.arrays[defcolor_list_item_active]      = make_colors(arena, 0xFF404040);
    default_color_table.arrays[defcolor_cursor]                = make_colors(arena, 0xFFFF6940);
    default_color_table.arrays[defcolor_at_cursor]             = make_colors(arena, 0xFF161616);
    default_color_table.arrays[defcolor_highlight_cursor_line] = make_colors(arena, 0xFF1E1612);
    default_color_table.arrays[defcolor_highlight]             = make_colors(arena, 0xFF703419);
    default_color_table.arrays[defcolor_at_highlight]          = make_colors(arena, 0xFFCDAA7D);
    default_color_table.arrays[defcolor_mark]                  = make_colors(arena, 0xFF808080);
    default_color_table.arrays[defcolor_text_default]          = make_colors(arena, 0xFFA08563);
    default_color_table.arrays[defcolor_comment]               = make_colors(arena, 0xFF7D7D7D);
    default_color_table.arrays[defcolor_keyword]               = make_colors(arena, 0xFFCD950C);
    default_color_table.arrays[defcolor_str_constant]          = make_colors(arena, 0xFF6B8E23);
    default_color_table.arrays[defcolor_char_constant]         = make_colors(arena, 0xFF6B8E23);
    default_color_table.arrays[defcolor_int_constant]          = make_colors(arena, 0xFF6B8E23);
    default_color_table.arrays[defcolor_float_constant]        = make_colors(arena, 0xFF6B8E23);
    default_color_table.arrays[defcolor_bool_constant]         = make_colors(arena, 0xFF6B8E23);
    default_color_table.arrays[defcolor_include]               = make_colors(arena, 0xFF6B8E23);
    default_color_table.arrays[defcolor_preproc]               = make_colors(arena, 0xFFDAB98F);
    default_color_table.arrays[defcolor_special_character]     = make_colors(arena, 0xFFFF0000);
    default_color_table.arrays[defcolor_ghost_character]       = make_colors(arena, 0xFF5B4D3C);
    default_color_table.arrays[defcolor_highlight_junk]        = make_colors(arena, 0xFF3A0000);
    default_color_table.arrays[defcolor_highlight_white]       = make_colors(arena, 0xFF003A3A);
    default_color_table.arrays[defcolor_paste]                 = make_colors(arena, 0xFFFFBB00);
    default_color_table.arrays[defcolor_undo]                  = make_colors(arena, 0xFF80005D);
    
    default_color_table.arrays[defcolor_back_cycle] = make_colors(arena, 0x0CA00000, 0x0800A000, 0x080000A0, 0x08A0A000);
    default_color_table.arrays[defcolor_text_cycle] = make_colors(arena, 0xFFA00000, 0xFF00A000, 0xFF0020B0, 0xFFA0A000);
    
    active_color_table = default_color_table;
}

void
SoimnRenderBuffer(Application_Links* app, View_ID view_id, Face_ID face_id,
                  Buffer_ID buffer, Text_Layout_ID text_layout_id,
                  Rect_f32 rect)
{
    ProfileScope(app, "SoimnRenderBuffer");
    
    View_ID active_view = get_active_view(app, Access_Always);
    bool is_active_view = (active_view == view_id);
    
    Range_i64 visible_range = text_layout_get_visible_range(app, text_layout_id);
    Face_Metrics metrics    = get_face_metrics(app, face_id);
    
    // NOTE(allen): Token colorizing
    Token_Array token_array = get_token_array_from_buffer(app, buffer);
    if (token_array.tokens != 0)
    {
        Scratch_Block scratch(app);
        
        i64 first_index         = token_index_from_pos(&token_array, visible_range.first);
        Token_Iterator_Array it = token_iterator_index(0, &token_array, first_index);
        for (;;)
        {
            Temp_Memory_Block temp(scratch);
            
            Token* token = token_it_read(&it);
            
            if (token->pos >= visible_range.one_past_last) break;
            else
            {
                Managed_ID color = defcolor_text_default;
                
                switch (token->kind)
                {
                    case TokenBaseKind_Preprocessor:   color = defcolor_preproc;        break;
                    case TokenBaseKind_Keyword:        color = defcolor_keyword;        break;
                    case TokenBaseKind_Comment:        color = defcolor_comment;        break;
                    case TokenBaseKind_LiteralString:  color = defcolor_str_constant;   break;
                    case TokenBaseKind_LiteralInteger: color = defcolor_int_constant;   break;
                    case TokenBaseKind_LiteralFloat:   color = defcolor_float_constant; break;
                }
                
                if (token->kind != TokenBaseKind_Identifier)
                {
                    switch (token->sub_kind)
                    {
                        case TokenCppKind_LiteralTrue:
                        case TokenCppKind_LiteralFalse:
                        color = defcolor_bool_constant;
                        break;
                        
                        case TokenCppKind_LiteralCharacter:
                        case TokenCppKind_LiteralCharacterWide:
                        case TokenCppKind_LiteralCharacterUTF8:
                        case TokenCppKind_LiteralCharacterUTF16:
                        case TokenCppKind_LiteralCharacterUTF32:
                        color = defcolor_char_constant;
                        break;
                        
                        case TokenCppKind_PPIncludeFile:
                        color = defcolor_include;
                        break;
                    }
                }
                
                ARGB_Color argb_color = fcolor_resolve(fcolor_id(color));
                
                if (token->kind == TokenBaseKind_Identifier)
                {
                    ProfileScope(app, "Code Note Array Lookup");
                    
                    String_Const_u8 lexeme = push_token_lexeme(app, scratch, buffer, token);
                    
                    Code_Note* note = 0;
                    
                    auto FirstCharIsValid = [](String_Const_u8 s) -> bool {
                        return (s.str[0] >= 'a' && s.str[0] <= 'z' ||
                                s.str[0] >= 'A' && s.str[0] <= 'Z' ||
                                s.str[0] == '_');
                    };
                    
                    if (FirstCharIsValid(lexeme))
                    {
                        for (Code_Note_Array* scan = NoteArrays[lexeme.str[0]];
                             scan != 0;
                             scan = scan->next)
                        {
                            for (Code_Note* note_scan = (Code_Note*)(scan + 1);
                                 note_scan < (Code_Note*)(scan + 1) + scan->size;
                                 ++note_scan)
                            {
                                if (string_match(lexeme, note_scan->text))
                                {
                                    note = note_scan;
                                    break;
                                }
                            }
                            
                            if (note != 0) break;
                        }
                    }
                    
                    if (note != 0)
                    {
                        if      (note->note_kind == CodeIndexNote_Type)     argb_color = SoimnTypeColor;
                        else if (note->note_kind == CodeIndexNote_Function) argb_color = SoimnFunctionColor;
                        else if (note->note_kind == CodeIndexNote_Macro)    argb_color = SoimnMacroColor;
                    }
                }
                
                paint_text_color(app, text_layout_id, Ii64_size(token->pos, token->size), argb_color);
                
                if (token->kind == TokenBaseKind_Comment)
                {
                    struct { String string; ARGB_Color color; } keywords[] = {
                        {CONST_STRING("NOTE"),      SoimnNoteColor},
                        {CONST_STRING("TODO"),      SoimnTodoColor},
                        {CONST_STRING("IMPORTANT"), SoimnImportantColor},
                        {CONST_STRING("HACK"),      SoimnHackColor},
                        {CONST_STRING("TEMP"),      SoimnTempColor},
                    };
                    
                    String comment_string = {0};
                    comment_string.data = push_array(scratch, U8, token->size);
                    comment_string.size = token->size;
                    
                    if (buffer_read_range(app, buffer, Ii64(token), comment_string.data))
                    {
                        if (SubStringCompare(comment_string, CONST_STRING("//// ")) &&
                            comment_string.size >= 5)
                        {
                            paint_text_color(app, text_layout_id, Ii64(token), SoimnErrCommentColor);
                            comment_string.data += 5;
                            comment_string.size -= 5;
                        }
                        
                        else if (SubStringCompare(comment_string, CONST_STRING("/// ")) &&
                                 comment_string.size >= 4)
                        {
                            paint_text_color(app, text_layout_id, Ii64(token), SoimnHighCommentColor);
                            comment_string.data += 4;
                            comment_string.size -= 4;
                        }
                        
                        
                        U32 offset = 0;
                        while (offset != comment_string.size)
                        {
                            String tail;
                            tail.data = comment_string.data + offset;
                            tail.size = comment_string.size - offset;
                            
                            bool did_match = false;
                            for (U32 i = 0; i < ArrayCount(keywords); ++i)
                            {
                                if (SubStringCompare(tail, keywords[i].string) &&
                                    tail.size >= keywords[i].string.size)
                                {
                                    paint_text_color(app, text_layout_id,
                                                     Ii64_size(token->pos + offset, keywords[i].string.size),
                                                     keywords[i].color);
                                    
                                    offset += (U32)keywords[i].string.size;
                                    
                                    did_match = true;
                                    break;
                                }
                            }
                            
                            if (!did_match) offset += 1;
                        }
                    }
                }
                
                if (!token_it_inc_all(&it)) break;
            }
        }
    }
    
    else
    {
        paint_text_color_fcolor(app, text_layout_id, visible_range, fcolor_id(defcolor_text_default));
    }
    
    I64 cursor_pos = view_correct_cursor(app, view_id);
    view_correct_mark(app, view_id);
    
    // NOTE(allen): Scope highlight
    if (global_config.use_scope_highlight)
    {
        Color_Array colors = finalize_color_array(defcolor_back_cycle);
        draw_enclosures(app, text_layout_id, buffer, cursor_pos, FindNest_Scope,
                        RangeHighlightKind_LineHighlight, colors.vals, colors.count, 0, 0);
    }
    
    if (global_config.use_error_highlight || global_config.use_jump_highlight)
    {
        // NOTE(allen): Error highlight
        String_Const_u8 name         = string_u8_litexpr("*compilation*");
        Buffer_ID compilation_buffer = get_buffer_by_name(app, name, Access_Always);
        
        if (global_config.use_error_highlight)
        {
            draw_jump_highlights(app, buffer, text_layout_id, compilation_buffer,
                                 fcolor_id(defcolor_highlight_junk));
        }
        
        // NOTE(allen): Search highlight
        if (global_config.use_jump_highlight)
        {
            Buffer_ID jump_buffer = get_locked_jump_buffer(app);
            
            if (jump_buffer != compilation_buffer)
            {
                draw_jump_highlights(app, buffer, text_layout_id, jump_buffer,
                                     fcolor_id(defcolor_highlight_white));
            }
        }
    }
    
    // NOTE(allen): Color parens
    Color_Array colors = finalize_color_array(defcolor_text_cycle);
    draw_paren_highlight(app, buffer, text_layout_id, cursor_pos, colors.vals, colors.count);
    
    // NOTE(allen): Line highlight
    i64 line_number = get_line_number_from_pos(app, buffer, cursor_pos);
    draw_line_highlight(app, text_layout_id, line_number,
                        fcolor_id(defcolor_highlight_cursor_line));
    
    // NOTE(soimn): Draw cursor
    bool has_highlight_range = draw_highlight_range(app, view_id, buffer, text_layout_id, 0);
    
    if (!has_highlight_range)
    {
        I32 cursor_sub_id = default_cursor_sub_id();
        
        I64 view_cursor_pos = view_get_cursor_pos(app, view_id);
        I64 view_mark_pos   = view_get_mark_pos(app, view_id);
        
        F32 outline_thickness = 2;
        
        if (is_active_view)
        {
            draw_character_block(app, text_layout_id, view_cursor_pos, 0,
                                 fcolor_id(defcolor_cursor, cursor_sub_id));
            
            paint_text_color_pos(app, text_layout_id, view_cursor_pos,
                                 fcolor_id(defcolor_at_cursor));
            
            draw_character_wire_frame(app, text_layout_id, view_mark_pos,
                                      0, outline_thickness,
                                      fcolor_id(defcolor_mark));
        }
        
        else
        {
            draw_character_wire_frame(app, text_layout_id, view_mark_pos,
                                      0, outline_thickness,
                                      fcolor_id(defcolor_mark));
            
            draw_character_wire_frame(app, text_layout_id, view_cursor_pos,
                                      0, outline_thickness,
                                      fcolor_id(defcolor_cursor, cursor_sub_id));
        }
    }
    
    // NOTE(allen): Fade ranges
    paint_fade_ranges(app, text_layout_id, buffer);
    
    // NOTE(allen): put the actual text on the actual screen
    draw_text_layout_default(app, text_layout_id);
}

void SoimnUpdateNoteArrays(Buffer_ID buffer);

void
SoimnRenderCaller(Application_Links* app, Frame_Info frame_info, View_ID view_id)
{
    ProfileScope(app, "SoimnRenderCaller");
    
    // NOTE(soimn): Ensure NoteArrays are initalized
    if (!NoteArraysInitialized)
    {
        for (Buffer_ID buffer_scan = get_buffer_next(app, 0, Access_Always);
             buffer_scan != 0;
             buffer_scan = get_buffer_next(app, buffer_scan, Access_Always))
        {
            SoimnUpdateNoteArrays(buffer_scan);
        }
        
        NoteArraysInitialized = true;
    }
    
    View_ID active_view = get_active_view(app, Access_Always);
    bool is_active_view = (active_view == view_id);
    
    // NOTE(soimn): Draw margin & background and set the clipping mask to be within the margin
    Rect_f32 region    = draw_background_and_margin(app, view_id, is_active_view);
    Rect_f32 prev_clip = draw_set_clip(app, region);
    
    // NOTE(soimn): Get target view pos and interpolate towards it with the current scroll rule
    Buffer_ID buffer                = view_get_buffer(app, view_id, Access_Always);
    Buffer_Scroll scroll            = view_get_buffer_scroll(app, view_id);
    Buffer_Point_Delta_Result delta = delta_apply(app, view_id, frame_info.animation_dt, scroll);
    
    if (!block_match_struct(&scroll.position, &delta.point))
    {
        block_copy_struct(&scroll.position, &delta.point);
        view_set_buffer_scroll(app, view_id, scroll, SetBufferScroll_NoCursorChange);
    }
    
    if (delta.still_animating) animate_in_n_milliseconds(app, 0);
    
    
    Face_ID face_id           = get_face_id(app, buffer);
    Face_Metrics face_metrics = get_face_metrics(app, face_id);
    F32 line_height           = face_metrics.line_height;
    
    // NOTE(allen): query bars
    Rect_f32 query_bar_region = region;
    
    Query_Bar* space[32];
    Query_Bar_Ptr_Array query_bars = {};
    query_bars.ptrs                = space;
    
    if (get_active_query_bars(app, view_id, ArrayCount(space), &query_bars))
    {
        Scratch_Block scratch(app);
        
        for (I32 i = 0; i < query_bars.count; ++i)
        {
            Rect_f32_Pair pair = rect_split_top_bottom(query_bar_region, line_height + 2);
            
            Fancy_Line list = {};
            push_fancy_string(scratch, &list, fcolor_id(defcolor_pop1), query_bars.ptrs[i]->prompt);
            push_fancy_string(scratch, &list, fcolor_id(defcolor_text_default), query_bars.ptrs[i]->string);
            
            draw_fancy_line(app, face_id, fcolor_zero(), &list, pair.min.p0 + V2f32(2, 2));
            
            query_bar_region = pair.max;
        }
    }
    
    draw_set_clip(app, query_bar_region);
    
    // NOTE(allen): FPS hud
    if (show_fps_hud)
    {
        Rect_f32_Pair pair = layout_fps_hud_on_bottom(region, line_height);
        draw_fps_hud(app, frame_info, face_id, pair.max);
        region = pair.min;
        animate_in_n_milliseconds(app, 1000);
    }
    
    // NOTE(allen): begin buffer render
    Buffer_Point buffer_point = scroll.position;
    Text_Layout_ID text_layout_id = text_layout_create(app, buffer, region, buffer_point);
    
    // NOTE(soimn): The clipping mask is set to query_bar_region, but the buffer region is set to region.
    //              This is done to make the query bar appear directly over, and occluding the buffer content,
    //              instead of offseting it to make space.
    SoimnRenderBuffer(app, view_id, face_id, buffer, text_layout_id, region);
    
    text_layout_free(app, text_layout_id);
    draw_set_clip(app, prev_clip);
}

void
SoimnWholeScreenRenderCaller(Application_Links* app, Frame_Info frame_info){
#if 0
    Rect_f32 region = global_get_screen_rectangle(app);
    Vec2_f32 center = rect_center(region);
    
    Face_ID face_id = get_face_id(app, 0);
    Scratch_Block scratch(app);
    draw_string_oriented(app, face_id, finalize_color(defcolor_text_default, 0),
                         SCu8("Hello, World!"), center - V2f32(200.f, 300.f),
                         0, V2f32(0.f, -1.f));
    draw_string_oriented(app, face_id, finalize_color(defcolor_text_default, 0),
                         SCu8("Hello, World!"), center - V2f32(240.f, 300.f),
                         0, V2f32(0.f, 1.f));
    draw_string_oriented(app, face_id, finalize_color(defcolor_text_default, 0),
                         SCu8("Hello, World!"), center - V2f32(400.f, 400.f),
                         0, V2f32(-1.f, 0.f));
    draw_string_oriented(app, face_id, finalize_color(defcolor_text_default, 0),
                         SCu8("Hello, World!"), center - V2f32(400.f, -100.f),
                         0, V2f32(cos_f32(pi_f32*.333f), sin_f32(pi_f32*.333f)));
#endif
}

BUFFER_HOOK_SIG(SoimnFileSave){
    ProfileScope(app, "SoimnFileSave");
    
    clean_all_lines_buffer(app, buffer_id, CleanAllLinesMode_LeaveBlankLines);
    
    bool is_virtual = global_config.enable_virtual_whitespace;
    if (global_config.automatically_indent_text_on_save && is_virtual)
    {
        auto_indent_buffer(app, buffer_id, buffer_range(app, buffer_id));
    }
    
    Managed_Scope scope   = buffer_get_managed_scope(app, buffer_id);
    Line_Ending_Kind* eol = scope_attachment(app, scope, buffer_eol_setting,
                                             Line_Ending_Kind);
    
    if (*eol == LineEndingKind_LF) rewrite_lines_to_lf(app, buffer_id);
    else                           rewrite_lines_to_crlf(app, buffer_id);
    
    return 0;
}

BUFFER_HOOK_SIG(SoimnNewFile)
{
    return 0;
}

BUFFER_HOOK_SIG(SoimnBeginBuffer)
{
    ProfileScope(app, "SoimnBeginBuffer");
    
    Scratch_Block scratch(app);
    
    b32 treat_as_code = false;
    String_Const_u8 file_name = push_buffer_file_name(app, scratch, buffer_id);
    if (file_name.size > 0)
    {
        String_Const_u8_Array extensions = global_config.code_exts;
        String_Const_u8 ext              = string_file_extension(file_name);
        
        for (i32 i = 0; i < extensions.count; ++i)
        {
            if (string_match(ext, extensions.strings[i]))
            {
                
                if (string_match(ext, string_u8_litexpr("cpp")) ||
                    string_match(ext, string_u8_litexpr("h"))   ||
                    string_match(ext, string_u8_litexpr("c"))   ||
                    string_match(ext, string_u8_litexpr("hpp")) ||
                    string_match(ext, string_u8_litexpr("cc")))
                {
                    treat_as_code = true;
                }
                
#if 0
                treat_as_code = true;
                
                if (string_match(ext, string_u8_litexpr("cs"))){
                    if (parse_context_language_cs == 0){
                        init_language_cs(app);
                    }
                    parse_context_id = parse_context_language_cs;
                }
                
                if (string_match(ext, string_u8_litexpr("java"))){
                    if (parse_context_language_java == 0){
                        init_language_java(app);
                    }
                    parse_context_id = parse_context_language_java;
                }
                
                if (string_match(ext, string_u8_litexpr("rs"))){
                    if (parse_context_language_rust == 0){
                        init_language_rust(app);
                    }
                    parse_context_id = parse_context_language_rust;
                }
                
                if (string_match(ext, string_u8_litexpr("cpp")) ||
                    string_match(ext, string_u8_litexpr("h")) ||
                    string_match(ext, string_u8_litexpr("c")) ||
                    string_match(ext, string_u8_litexpr("hpp")) ||
                    string_match(ext, string_u8_litexpr("cc"))){
                    if (parse_context_language_cpp == 0){
                        init_language_cpp(app);
                    }
                    parse_context_id = parse_context_language_cpp;
                }
                
                // TODO(NAME): Real GLSL highlighting
                if (string_match(ext, string_u8_litexpr("glsl"))){
                    if (parse_context_language_cpp == 0){
                        init_language_cpp(app);
                    }
                    parse_context_id = parse_context_language_cpp;
                }
                
                // TODO(NAME): Real Objective-C highlighting
                if (string_match(ext, string_u8_litexpr("m"))){
                    if (parse_context_language_cpp == 0){
                        init_language_cpp(app);
                    }
                    parse_context_id = parse_context_language_cpp;
                }
#endif
                
                break;
            }
        }
    }
    
    Command_Map_ID map_id = (treat_as_code ? mapid_code : mapid_file);
    Managed_Scope scope   = buffer_get_managed_scope(app, buffer_id);
    
    Command_Map_ID* map_id_ptr = scope_attachment(app, scope, buffer_map_id, Command_Map_ID);
    *map_id_ptr = map_id;
    
    Line_Ending_Kind setting      = guess_line_ending_kind_from_buffer(app, buffer_id);
    Line_Ending_Kind* eol_setting = scope_attachment(app, scope, buffer_eol_setting, Line_Ending_Kind);
    *eol_setting = setting;
    
    // NOTE(allen): Decide buffer settings
    b32 wrap_lines = true;
    b32 use_lexer = false;
    if (treat_as_code)
    {
        wrap_lines = global_config.enable_code_wrapping;
        use_lexer = true;
    }
    
    String_Const_u8 buffer_name = push_buffer_base_name(app, scratch, buffer_id);
    if (buffer_name.size > 0 && buffer_name.str[0] == '*' && buffer_name.str[buffer_name.size - 1] == '*')
    {
        wrap_lines = global_config.enable_output_wrapping;
    }
    
    if (use_lexer)
    {
        ProfileBlock(app, "begin buffer kick off lexer");
        Async_Task* lex_task_ptr = scope_attachment(app, scope, buffer_lex_task, Async_Task);
        *lex_task_ptr = async_task_no_dep(&global_async_system, do_full_lex_async, make_data_struct(&buffer_id));
    }
    
    {
        b32* wrap_lines_ptr = scope_attachment(app, scope, buffer_wrap_lines, b32);
        *wrap_lines_ptr = wrap_lines;
    }
    
    if (use_lexer)
    {
        buffer_set_layout(app, buffer_id, layout_virt_indent_index_generic);
    }
    
    else
    {
        if (treat_as_code)
        {
            buffer_set_layout(app, buffer_id, layout_virt_indent_literal_generic);
        }
        
        else
        {
            buffer_set_layout(app, buffer_id, layout_generic);
        }
    }
    
    // NOTE(soimn): Add buffer info
    
    bool add_new = (buffer_id >= BufferInfoCount);
    if (BufferInfoCount == BufferInfoCapacity)
    {
        BufferInfoCapacity = MAX(BufferInfoCapacity * 2, 128);
        
        void* memory = malloc((alignof(void*) - 1) + sizeof(void*) + sizeof(Buffer_Info) * BufferInfoCapacity);
        
        if (BufferInfoCount != 0)
        {
            Copy(BufferInfos, memory, sizeof(Buffer_Info) * BufferInfoCapacity);
            
            free((U8*)BufferInfos - sizeof(void*));
        }
        
        BufferInfos = (Buffer_Info*)memory;
    }
    
    ASSERT(buffer_id <= BufferInfoCapacity);
    
    Buffer_Info* info = &BufferInfos[buffer_id];
    ZeroStruct(info);
    
    if (add_new) BufferInfoCount += 1;
    
    return 0;
}

void SoimnRemoveNoteArrays(Buffer_Info* info);
BUFFER_HOOK_SIG(SoimnEndBuffer)
{
    Buffer_Info* info = &BufferInfos[buffer_id];
    SoimnRemoveNoteArrays(info);
    
    Marker_List *list = get_marker_list_for_buffer(buffer_id);
    
    if (list != 0)
    {
        delete_marker_list(list);
    }
    
    default_end_buffer(app, buffer_id);
    
    return 0;
}

void
SoimnRemoveNoteArrays(Buffer_Info* info)
{
    for (U32 i = 0; i < ArrayCount(info->note_arrays); ++i)
    {
        if (info->note_arrays[i] == 0) continue;
        
        Code_Note_Array* prev = 0;
        for (Code_Note_Array* scan = NoteArrays[i]; scan != 0; )
        {
            if (scan == info->note_arrays[i]) break;
            
            prev = scan;
            scan = scan->next;
        }
        
        if (prev) prev->next = info->note_arrays[i]->next;
        else NoteArrays[i]   = info->note_arrays[i];
    }
    
    Arena_FreeAll(&info->arena);
}

void
SoimnUpdateNoteArrays(Buffer_ID buffer)
{
    ASSERT(buffer <= BufferInfoCount);
    
    Buffer_Info* info = &BufferInfos[buffer];
    SoimnRemoveNoteArrays(info);
    
    Code_Index_File* file = code_index_get_file(buffer);
    
    if (file != 0)
    {
        for (I32 i = 0; i < file->note_array.count; ++i)
        {
            auto FirstCharIsValid = [](String_Const_u8 s) -> bool {
                return (s.str[0] >= 'a' && s.str[0] <= 'z' ||
                        s.str[0] >= 'A' && s.str[0] <= 'Z' ||
                        s.str[0] == '_');
            };
            
            ASSERT(FirstCharIsValid(file->note_array.ptrs[i]->text));
            
            Code_Note_Array** array = &info->note_arrays[i];
            
            if (*array == 0 || (*array)->size == (*array)->capacity)
            {
                U64 new_capacity = (*array == 0 ? 256 : 2 * (*array)->capacity);
                auto new_array = (Code_Note_Array*)Arena_Allocate(&info->arena,
                                                                  sizeof(Code_Note_Array) + sizeof(Code_Note) * new_capacity,
                                                                  alignof(Code_Note_Array));
                
                new_array->next     = 0;
                new_array->size     = 0;
                new_array->capacity = new_capacity;
                
                if (*array != 0)
                {
                    Copy((U8*)*array + sizeof(Code_Note_Array), (U8*)new_array + sizeof(Code_Note_Array),
                         (*array)->size * sizeof(Code_Note));
                    
                    new_array->size = (*array)->size;
                }
                
                if (*array != 0) new_array->next = (*array)->next;
                else             new_array->next = NoteArrays[i];
                
                *array        = new_array;
                NoteArrays[i] = new_array;
            }
            
            Code_Note* note = (Code_Note*)(info->note_arrays[i] + 1) + info->note_arrays[i]->size;
            info->note_arrays[i]->size += 1;
            
            Code_Index_Note* index_note = file->note_array.ptrs[i];
            
            note->note_kind = index_note->note_kind;
            note->pos       = index_note->pos;
            note->text      = index_note->text;
            note->file      = index_note->file;
            note->peek[0]   = index_note->text.str[0];
            note->peek[1]   = (index_note->text.size > 1 ? index_note->text.str[1] : 0);
        }
    }
}

void
SoimnTick(Application_Links *app, Frame_Info frame_info)
{
    //code_index_update_tick(app);
    Scratch_Block scratch(app);
    
    for (Buffer_Modified_Node *node = global_buffer_modified_set.first;
         node != 0;
         node = node->next)
    {
        Temp_Memory_Block temp(scratch);
        Buffer_ID buffer_id = node->buffer;
        
        String_Const_u8 contents = push_whole_buffer(app, scratch, buffer_id);
        Token_Array tokens = get_token_array_from_buffer(app, buffer_id);
        
        if (tokens.count == 0) continue;
        
        Arena arena = make_arena_system(KB(16));
        
        Code_Index_File* index = push_array_zero(&arena, Code_Index_File, 1);
        index->buffer = buffer_id;
        
        Generic_Parse_State state = {};
        generic_parse_init(app, &arena, contents, &tokens, &state);
        
        state.do_cpp_parse = true;
        generic_parse_full_input_breaks(index, &state, max_i32);
        
        code_index_lock();
        code_index_set_file(buffer_id, arena, index);
        
        SoimnUpdateNoteArrays(buffer_id);
        
        code_index_unlock();
        
        buffer_clear_layout_cache(app, buffer_id);
    }
    
    buffer_modified_set_clear();
    
    if (tick_all_fade_ranges(app, frame_info.animation_dt))
    {
        animate_in_n_milliseconds(app, 0);
    }
}

void
custom_layer_init(Application_Links* app)
{
    Thread_Context* tctx = get_thread_context(app);
    
    {
        async_task_handler_init(app, &global_async_system);
        clipboard_init(get_base_allocator_system(), /*history_depth*/ 64, &clipboard0);
        code_index_init();
        buffer_modified_set_init();
        Profile_Global_List* list = get_core_profile_list(app);
        ProfileThreadName(tctx, list, string_u8_litexpr("main"));
        initialize_managed_id_metadata(app);
        mapid_delete = managed_id_declare(app, string_u8_litexpr("command_map"), string_u8_litexpr("mapid_delete"));
        set_default_color_scheme(app);
        heap_init(&global_heap, tctx->allocator);
        global_config_arena = make_arena_system();
        fade_range_arena = make_arena_system(KB(8));
    }
    
    {
        set_custom_hook(app, HookID_BufferViewerUpdate,      default_view_adjust);
        set_custom_hook(app, HookID_ViewEventHandler,        default_view_input_handler);
        set_custom_hook(app, HookID_Tick,                    SoimnTick);
        set_custom_hook(app, HookID_RenderCaller,            SoimnRenderCaller);
        set_custom_hook(app, HookID_WholeScreenRenderCaller, SoimnWholeScreenRenderCaller);
        
        set_custom_hook(app, HookID_DeltaRule, original_delta);
        set_custom_hook_memory_size(app, HookID_DeltaRule,
                                    delta_ctx_size(original_delta_memory_size));
        
        set_custom_hook(app, HookID_BufferNameResolver, default_buffer_name_resolution);
        
        set_custom_hook(app, HookID_BeginBuffer,      SoimnBeginBuffer);
        set_custom_hook(app, HookID_EndBuffer,        SoimnEndBuffer);
        set_custom_hook(app, HookID_NewFile,          SoimnNewFile);
        set_custom_hook(app, HookID_SaveFile,         SoimnFileSave);
        set_custom_hook(app, HookID_BufferEditRange,  default_buffer_edit_range);
        set_custom_hook(app, HookID_BufferRegion,     default_buffer_region);
        set_custom_hook(app, HookID_ViewChangeBuffer, default_view_change_buffer);
        
        set_custom_hook(app, HookID_Layout, layout_unwrapped);
    }
    
    mapping_init(tctx, &framework_mapping);
    
    SoimnBindKeys(&framework_mapping);
    
    global_config.lister_roundness = 0;
    SoimnSetColors(app);
}

#endif

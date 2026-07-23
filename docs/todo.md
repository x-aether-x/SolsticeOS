# Setup a WM

Todo:
 - make fill_rect, draw_rect, blit, draw_char functions in gfx.cpp ✅
 - setup double buffering so that whenever i want to redraw something my screen doesnt flicker ✅
 - draw the cursor (always last thing drawn) ✅
 - window struct, wm_create_window should return a handle
 - compositor loop using scheduler
 - focusing windows and keyboard routing
 - client drawing api, so that window processes can have a way to draw in the client based buffer
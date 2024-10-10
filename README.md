# gl-letterbox
Letterbox utility implementation for OpenGL.

## Features
- Letterboxing screen
- Calculating virtual mouse position

## Usage
1. Create `gllb::LetterboxManager` with initial window size.
2. Call `gllb::LetterboxManager::on_window_resized()` method when window has been resized.
3. Fetch screen scale using `gllb::LetterboxManager::get_screen_scale()` for every window resized event.
4. Draw pre-drawn screen using frame buffer with screen scale.
5. Fetch and store virtual mouse position using `gllb::LetterboxManager::get_virtual_mouse_position()` with  
   origin mouse position when mouse position has been changed.

## Example
See [example.cpp](/example.cpp).  
You should setup FBO, window resizing callback and mouse position callback manually. 

#ifndef __GL_LETTERBOX_HPP__
#define __GL_LETTERBOX_HPP__

/*
	gl-letterbox

	Letterbox utility implementation for OpenGL.

	produced by scenent(https://github.com/scenent)
	distributed under Creative Commons Zero (CC0)
*/

#include <algorithm> // std::clamp()

namespace gllb {
	class LetterboxManager final {
	private:
		uint32_t m_originWindowWidth = 0U;
		uint32_t m_originWindowHeight = 0U;
		uint32_t m_lastWindowWidth = 0U;
		uint32_t m_lastWindowHeight = 0U;
	private:
		float    m_scaleX = 1.0f;
		float    m_scaleY = 1.0f;
	private:
		enum class LetterboxAlign {
			None, Width, Height
		} m_align = LetterboxAlign::None;
	public:
		LetterboxManager() = delete;
		LetterboxManager(const uint32_t& _initial_width, const uint32_t& _initial_height)
			: m_originWindowWidth(_initial_width),
			m_originWindowHeight(_initial_height),
			m_lastWindowWidth(_initial_width),
			m_lastWindowHeight(_initial_height)
		{

		}
		~LetterboxManager() = default;
		LetterboxManager(const LetterboxManager&) = delete;
		LetterboxManager(LetterboxManager&&) = delete;
		const LetterboxManager& operator=(const LetterboxManager&) = delete;
		const LetterboxManager& operator=(LetterboxManager&&) = delete;
	public:
		// Call this method when window has been resized.
		void on_window_resized(
			const uint32_t& _current_width,
			const uint32_t& _current_height
		)
		{
			float _finalWidth = 0.0f;
			float _finalHeight = 0.0f;

			if (m_lastWindowWidth != _current_width) {
				if (m_originWindowWidth > _current_width) {
					if ((m_originWindowWidth > m_originWindowHeight) && _current_width < _current_height) {
						m_scaleX = 1.0f;
						_finalWidth = _current_width;
						_finalHeight = (m_originWindowHeight * _finalWidth) / m_originWindowWidth;
						m_scaleY = (_finalHeight / _current_height);
						m_align = LetterboxAlign::Width;
					}
					else {
						m_scaleY = 1.0f;
						_finalHeight = _current_height;
						_finalWidth = (m_originWindowWidth * _finalHeight) / m_originWindowHeight;
						m_scaleX = (_finalWidth / _current_width);
						m_align = LetterboxAlign::Height;
						if ((m_scaleX >= 1.0f) && (m_scaleY >= 1.0f)) {
							m_scaleX = 1.0f;
							_finalWidth = _current_width;
							_finalHeight = (m_originWindowHeight * _finalWidth) / m_originWindowWidth;
							m_scaleY = (_finalHeight / _current_height);
							m_align = LetterboxAlign::Width;
						}
					}
				}
				else if (m_originWindowWidth <= _current_width) {
					if ((m_originWindowWidth > m_originWindowHeight) && _current_width < _current_height) {
						m_scaleX = 1.0f;
						_finalWidth = _current_width;
						_finalHeight = (m_originWindowHeight * _finalWidth) / m_originWindowWidth;
						m_scaleY = (_finalHeight / _current_height);
						m_align = LetterboxAlign::Width;
					}
					else {
						m_scaleY = 1.0f;
						_finalHeight = _current_height;
						_finalWidth = (m_originWindowWidth * _finalHeight) / m_originWindowHeight;
						m_scaleX = (_finalWidth / _current_width);
						m_align = LetterboxAlign::Height;
						if ((m_scaleX >= 1.0f) && (m_scaleY >= 1.0f)) {
							m_scaleX = 1.0f;
							_finalWidth = _current_width;
							_finalHeight = (m_originWindowHeight * _finalWidth) / m_originWindowWidth;
							m_scaleY = (_finalHeight / _current_height);
							m_align = LetterboxAlign::Width;
						}
					}
				}
			}
			else if (m_lastWindowHeight != _current_height) {
				if (m_originWindowHeight > _current_height) {
					if ((m_originWindowWidth > m_originWindowHeight) && _current_width < _current_height) {
						m_scaleX = 1.0f;
						_finalWidth = _current_width;
						_finalHeight = (m_originWindowHeight * _finalWidth) / m_originWindowWidth;
						m_scaleY = (_finalHeight / _current_height);
						m_align = LetterboxAlign::Width;
					}
					else {
						m_scaleX = 1.0f;
						_finalWidth = _current_width;
						_finalHeight = (m_originWindowHeight * _finalWidth) / m_originWindowWidth;
						m_scaleY = (_finalHeight / _current_height);
						m_align = LetterboxAlign::Width;
						if ((m_scaleX >= 1.0f) && (m_scaleY >= 1.0f)) {
							m_scaleY = 1.0f;
							_finalHeight = _current_height;
							_finalWidth = (m_originWindowWidth * _finalHeight) / m_originWindowHeight;
							m_scaleX = (_finalWidth / _current_width);
							m_align = LetterboxAlign::Height;
						}
					}
				}
				else if (m_originWindowHeight <= _current_height) {
					if ((m_originWindowWidth > m_originWindowHeight) && _current_width < _current_height) {
						m_scaleX = 1.0f;
						_finalWidth = _current_width;
						_finalHeight = (m_originWindowHeight * _finalWidth) / m_originWindowWidth;
						m_scaleY = (_finalHeight / _current_height);
						m_align = LetterboxAlign::Width;
					}
					else {
						m_scaleX = 1.0f;
						_finalWidth = _current_width;
						_finalHeight = (m_originWindowHeight * _finalWidth) / m_originWindowWidth;
						m_scaleY = (_finalHeight / _current_height);
						m_align = LetterboxAlign::Width;
						if ((m_scaleX >= 1.0f) && (m_scaleY >= 1.0f)) {
							m_scaleY = 1.0f;
							_finalHeight = _current_height;
							_finalWidth = (m_originWindowWidth * _finalHeight) / m_originWindowHeight;
							m_scaleX = (_finalWidth / _current_width);
							m_align = LetterboxAlign::Height;
						}
					}
				}
			}
			m_lastWindowWidth = _current_width;
			m_lastWindowHeight = _current_height;
		}
	public:
		void get_screen_scale(float& _out_width, float& _out_height) const {
			_out_width = m_scaleX;
			_out_height = m_scaleY;
		}
		void get_virtual_mouse_position(
			const double& _origin_x, 
			const double& _origin_y, 
			double& _out_x,
			double& _out_y
		) const 
		{
			switch (m_align) {
			case (LetterboxAlign::None): {
				_out_x = _origin_x;
				_out_y = _origin_y;
				break;
			}
			case (LetterboxAlign::Width): {
				double _margin = (double(m_lastWindowHeight) / 2.0) * (1.0 - m_scaleY);
				double _finalYPos = std::clamp(_origin_y, _margin, m_lastWindowHeight - _margin);
				_finalYPos -= _margin;
				_finalYPos = (_finalYPos / (m_lastWindowHeight - (_margin * 2.0))) * m_originWindowHeight;
				double _finalXPos = _origin_x / (static_cast<double>(m_lastWindowWidth) / static_cast<double>(m_originWindowWidth));
				_out_x = _finalXPos;
				_out_y = _finalYPos;
				break;
			}
			case (LetterboxAlign::Height): {
				double _margin = (double(m_lastWindowWidth) / 2.0) * (1.0 - m_scaleX);
				double _finalXPos = std::clamp(_origin_x, _margin, m_lastWindowWidth - _margin);
				_finalXPos -= _margin;
				_finalXPos = (_finalXPos / (m_lastWindowWidth - (_margin * 2.0))) * m_originWindowWidth;
				double _finalYPos = _origin_y / (static_cast<double>(m_lastWindowHeight) / static_cast<double>(m_originWindowHeight));
				_out_x = _finalXPos;
				_out_y = _finalYPos;
				break;
			}
			}
		}
	};
}

#endif

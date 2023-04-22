#pragma once

namespace san::blur {

// Horizontal or vertical line adaptor
class line_adaptor {
	uint32_t *	m_ptr;
	int			m_len;
	int			m_advance;

public:
	line_adaptor( uint32_t * p_src, int len, int advance ) :
		m_ptr( p_src ), m_len( len ), m_advance( advance ) {}

	uint32_t *	ptr()			{ return m_ptr; }
	int			advance() const	{ return m_advance; }

	uint32_t get_pix( int i ) const {
		if ( i < 0 ) i = 0; else if ( i >= m_len ) i = m_len - 1;
		return m_ptr[i * m_advance];
	}

	void set_pix( int i, uint32_t value ) {
		m_ptr[i * m_advance] = value;
	}
}; // class line_adaptor

} // namespace san::blur

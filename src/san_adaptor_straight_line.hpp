#pragma once

namespace san::adaptor {

// 32-bpp horizontal or vertical line adaptor
class straight_line {
	uint32_t *	m_ptr;
	uint32_t *	m_ptr_start;
	int			m_len;
	int			m_advance;

public:
	straight_line( uint32_t * p_src, int len, int advance ) :
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


	// This can speed up sequential writes...
	// 'set_pix_start' should be used with 'set_pix_next' or not used at all
	void set_pix_start( int i ) {
		m_ptr_start = m_ptr + i * m_advance;
	}

	void set_pix_next( uint32_t value ) {
		*m_ptr_start = value;
		m_ptr_start += m_advance;
	}
}; // class straight_line

} // namespace san::adaptor

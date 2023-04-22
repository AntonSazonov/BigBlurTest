#pragma once

#include <list>
#include <filesystem>
#include "stb_impl.hpp"

namespace san {

class image_list {
	std::list <std::shared_ptr <surface>>	m_images; // Image list from ./pics
	decltype(m_images)::const_iterator		m_image_curr;

public:
	image_list( const std::string & path = "./pics/" ) {

		if ( !std::filesystem::exists( path ) ) {
			fprintf( stderr, "The path '%s' doesn't exist.\n", path.c_str() );
			return;
		}

		// Load all JPEGs from diretory...
		for ( auto & p : std::filesystem::recursive_directory_iterator( path ) ) {
			if ( p.path().extension() == ".jpg" ) { // JPEGs only
				std::printf( "Loading '%s'...\n", p.path().string().c_str() );
				std::shared_ptr <surface> image = load_image( p.path().string().c_str() );
				if ( image ) {
					m_images.push_back( image );
				}
			}
		}

		// Select first image in list.
		if ( !m_images.empty() ) {
			m_image_curr = m_images.cbegin();
		}
	}

	explicit operator bool () const { return !m_images.empty(); }

	std::shared_ptr <surface> current_image() const { return *m_image_curr; }

	void go_prev() {
		if ( m_image_curr == m_images.cbegin() ) {
			m_image_curr = --m_images.cend();
		} else {
			--m_image_curr;
		}
	}

	void go_next() {
		++m_image_curr;
		if ( m_image_curr == m_images.cend() ) {
			m_image_curr = m_images.cbegin();
		}
	}

}; // class image_list

} // namespace san

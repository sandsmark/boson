/***************************************************************************
    LibUFO - UI For OpenGL
    copyright         : (C) 2001-2005 by Johannes Schmidt
    email             : schmidtjf at users.sourceforge.net
                             -------------------

    file              : src/uvertexarray.hpp
    begin             : Wed 27 Apr 2005
    $Id$
 ***************************************************************************/

/***************************************************************************
 *  This library is free software; you can redistribute it and/or          *
 * modify it under the terms of the GNU Lesser General Public              *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This library is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Lesser General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this library; if not, write to the Free Software     *
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
 ***************************************************************************/

#include "ufo/uvertexarray.hpp"

#include <utility>

using namespace ufo;

UFO_IMPLEMENT_CLASS(UVertexArray, UObject)

UVertexArray::UVertexArray()
	: m_type(NoType)
	, m_vertices()
	, m_colors()
	, m_array(NULL)
{}

UVertexArray::~UVertexArray() {
	dispose();
}

void
UVertexArray::setOffset(float x, float y) {
	// FIXME: to be implemented
}

void
UVertexArray::add(float x, float y) {
	m_vertices.push_back(std::make_pair(x, y));
	if (m_colors.size() && m_vertices.size() > m_colors.size()) {
		m_colors.push_back(m_colors[m_colors.size() - 1]);
	}
}

void
UVertexArray::setColor(const UColor & col) {
	if (m_colors.size() == 0 && m_vertices.size() > 0) {
		for (unsigned int i = 0; i < m_vertices.size(); ++i) {
			m_colors.push_back(UColor::black);
		}
	}
	m_colors.push_back(col);
}


int
UVertexArray::getCount() const {
	return m_vertices.size();
}

int
UVertexArray::getType() const {
	if (m_type == NoType) {
		if (m_colors.size()) {
			(const_cast<UVertexArray*>(this))->m_type = C3F_V3F;
		} else {
			(const_cast<UVertexArray*>(this))->m_type = V3F;
		}
	}
	return m_type;
}

void
UVertexArray::setType(Type t) {
	// ensure that we have a valid m_type
	getType();
	if (t != m_type) {
		m_type = t;
		dispose();
	}
}

void *
UVertexArray::getArray() {
	// ensure that we have a valid m_type
	getType();
	if (m_array) {
		return m_array;
	}

	// create a new array
	if (m_type == V3F) {
		float * ret = new float[getCount() * 3];
		for (int i = 0; i < getCount(); ++i) {
			ret[i * 3] = m_vertices[i].first;
			ret[i * 3 + 1] = m_vertices[i].second;
			ret[i * 3 + 2] = 0.0f;
		}
		m_array = ret;
	} else if (m_type == C3F_V3F) {
		float * ret = new float[getCount() * 6];
		for (int i = 0; i < getCount(); ++i) {
			ret[i * 6] = m_colors[i].getRed();
			ret[i * 6 + 1] = m_colors[i].getGreen();
			ret[i * 6 + 2] = m_colors[i].getBlue();
			ret[i * 6 + 3] = m_vertices[i].first;
			ret[i * 6 + 4] = m_vertices[i].second;
			ret[i * 6 + 5] = 0.0f;
		}
		m_array = ret;
	}
	// FIXME: Add support for other vertex types
	return m_array;
}

std::vector<std::pair<float, float> >
UVertexArray::getVertices() const {
	return m_vertices;
}

std::vector<UColor>
UVertexArray::getColors() const {
	return m_colors;
}

void
UVertexArray::dispose() {
	if (m_array) {
		// FIXME: We use only float arrays but anyhow this is a bit dirty
		delete[] (static_cast<float*>(m_array));
		m_array = NULL;
	}
}

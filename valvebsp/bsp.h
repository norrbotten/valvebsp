#pragma once

#include <unordered_map>
#include <vector>

namespace ValveBSP {

	// Number of lumps in the header
	constexpr size_t HEADER_LUMPS = 64;

	// The VBSP magic number, ie. the first 4 bytes of the file
	constexpr unsigned int VBSP_MAGIC_NUMBER = ('P' << 24) + ('S' << 16) + ('B' << 8) + 'V';

	constexpr unsigned int MAX_MAP_PLANES = 65536;	// Maximum amount of planes
	constexpr unsigned int MAX_MAP_VERTS = 65536;	// Maximum amount of vertexes
	constexpr unsigned int MAX_MAP_EDGES = 256000;	// Maximum amount of edges
	constexpr unsigned int MAX_MAP_SURFEDGES = 512000;	// Maximum amount of surfedges

	// 3D vector
	struct Vector {
		float x, y, z;

		void operator+(Vector rhs) {
			this->x += rhs.x; this->y += rhs.y; this->z += rhs.z;
		}

		void operator-(Vector rhs) {
			this->x -= rhs.x; this->y -= rhs.y; this->z -= rhs.z;
		}

		void operator*(float rhs) {
			this->x *= rhs; this->y *= rhs; this->z *= rhs;
		}

		inline float Length() {
			return sqrtf(this->x * this->x + this->y * this->y + this->z * this->z);
		}

		void Normalize() {
			float len = this->Length();
			if (len != 0) {
				this->x /= len; this->y /= len; this->z /= len;
			}
		}

		friend std::ostream& operator<<(std::ostream& os, Vector v) {
			os << "[" << v.x << ", " << v.y << ", " << v.z << "]";
			return os;
		}
	};

	// lump_t struct
	// Contains information about a specific lump, the lump_t structs
	//  index in the lump array defines its lump type
	struct lump_t {
		int		fileofs;	// Offset into the file
		int		filelen;	// Size of the lump in bytes
		int		version;	// Lump format version (???)
		char	fourCC[4];	// Lump ident code (???)
	};

	// dheader_t struct
	// Contains information about all lumps present in the file
	struct dheader_t {
		int		ident;				// BSP file identifier, should be VBSP
		int		version;			// BSP file version
		lump_t	lumps[HEADER_LUMPS];// Lump directory
		int		mapRevision;		// Maps "iteration" number, how many times the .vmf has been compiled
	};

	// dplane_t struct
	struct dplane_t {
		Vector	normal;
		float	dist;
		int		type;
	};

	// vertex, just a vector
	typedef Vector dvertex_t;

	// dedge_t, two shorts which reference the vertex lump
	struct dedge_t {
		unsigned short v[2];
	};

	// dsurfedge_t, just an integer
	typedef int dsurfedge_t;

	/*
		ALL LUMPS ARE DEFINED BELOW
	*/

	// LUMP 0, entity lump
	// This is an ASCII text buffer
	typedef std::string lump_entities;

	// LUMP 1, planes
	// array of dplane_t structs
	typedef std::vector<dplane_t> lump_planes;

	// LUMP 3, vertexes
	// array of dvertex_t
	typedef std::vector<dvertex_t> lump_vertexes;

	// LUMP 12, edges
	// array of dedge_t
	typedef std::vector<dedge_t> lump_edges;

	// LUMP 13, surfedges
	// array of dsurfedge_t
	typedef std::vector<dsurfedge_t> lump_surfedges;

	struct BSP {
		dheader_t header;

		// map used to reference parts of this struct
		std::unordered_map<int, void*> lumps;

		lump_entities	lump0_entities;
		lump_planes		lump1_planes;
		lump_vertexes	lump3_vertexes;
		lump_edges		lump12_edges;
		lump_surfedges	lump13_surfedges;
	};

}
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
	constexpr unsigned int MAX_MAP_FACES = 65536;	// Maximum amount of faces
	constexpr unsigned int MAX_MAP_TEXDATAS = 2048;	// Maximum amount of texture datas
	constexpr unsigned int MAX_MAP_NODES = 65536;	// Maximum amount of BSP nodes

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

	// dnode_t struct
	struct dnode_t {
		int				planenum;	// index into plane array
		int				children[2];// negative numbers are -(leafs + 1), not nodes
		short			mins[3];	// for frustum culling
		short			maxs[3];
		unsigned short	firstface;	// index into face array
		unsigned short	numfaces;	// counting both sides
		short			area;		// If all leaves below this node are in the same area, then
									// this is the area index. If not, this is -1.
		short			paddding;	// pad to 32 bytes length
	};

	// dleaf_t struct
	struct dleaf_t {
		int				contents;		// OR of all brushes (not needed?)
		short			cluster;		// cluster this leaf is in
		short			area:9;			// area this leaf is in
		short			flags:7;		// flags
		short			mins[3];		// for frustum culling
		short			maxs[3];
		unsigned short	firstleafface;	// index into leaffaces
		unsigned short	numleaffaces;
		unsigned short	firstleafbrush;	// index into leafbrushes
		unsigned short	numleafbrushes;
		short			leafWaterDataID;// -1 for not in water
	};

	struct ColorRGBExp32 {
		unsigned char r, g, b;
		char exponent;
	};

	struct CompressedLightCube {
		ColorRGBExp32 m_Color[6];
	};

	// dleaf_t struct for map versions 19 or earlier
	struct dleaf_t_19
	{
		int				contents;		// OR of all brushes (not needed?)
		short			cluster;		// cluster this leaf is in
		short			area:9;			// area this leaf is in
		short			flags:7;		// flags
		short			mins[3];		// for frustum culling
		short			maxs[3];
		unsigned short	firstleafface;	// index into leaffaces
		unsigned short	numleaffaces;
		unsigned short	firstleafbrush;	// index into leafbrushes
		unsigned short	numleafbrushes;
		short			leafWaterDataID;// -1 for not in water
		
		CompressedLightCube	ambientLighting;	// Precaculated light info for entities.
		short			padding;		// padding to 4-byte boundary
	};

	// dtexdata_t, texture data
	struct dtexdata_t {
		Vector	reflectivity;
		int		nameStringTableID;
		int		width;
		int		height;
		int		view_width;
		int		view_height;
	};

	// dvertex_t, just a vector
	typedef Vector dvertex_t;

	// dface_t
	struct dface_t {
		unsigned short	planenum;		// the plane number
		char			side;			// faces opposite to the node's plane direction
		char			onNode;			// 1 of on node, 0 if in leaf
		int				firstedge;		// index into surfedges
		short			numedges;		// number of surfedges
		short			texinfo;		// texture info
		short			dispinfo;		// displacement info
		char			surfaceFogVolumeID;	// ???
		char			styles[4];		// switchable lighting info
		int				lightofs;		// offset into lightmap lump
		float			area;			// face area in units^2
		int				LightmapTextureMinsInLuxels[2];	// texture lighting info
		int				LightmapTextureSizeInLuxels[2];	// texture lighting info
		int				origFace;		// original face this was split from
		unsigned short	numPrims;		// primitives
		unsigned short	firstPrimID;	// ???
		unsigned int	smoothingGroups;// lightmap smoothing group
	};

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

	// LUMP 2, texture data
	// array of dtexdata_t structs
	typedef std::vector<dtexdata_t> lump_texdata;

	// LUMP 3, vertexes
	// array of dvertex_t
	typedef std::vector<dvertex_t> lump_vertexes;

	// LUMP 5, bsp nodes
	// array of dnote_t
	typedef std::vector<dnode_t> lump_nodes;

	// LUMP 7, faces
	// array of dedge_t
	typedef std::vector<dface_t> lump_faces;



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
		lump_texdata	lump2_texdata;
		lump_vertexes	lump3_vertexes;
		lump_faces		lump7_faces;
		lump_edges		lump12_edges;
		lump_surfedges	lump13_surfedges;
	};

}
#pragma once

#include <fstream>

#include "bsp.h"

namespace ValveBSP {
	
	class Parser {
	private:
		char*	buffer = nullptr;
		size_t	position = 0;
		size_t	length = 0;

		bool	ready = false;
		bool	verbose = false;

		#define verboseprint(x) if (this->verbose) std::cout << x

		template<typename T>
		bool ReadType(T& t) {
			size_t size = sizeof(T);

			// check for EOF
			if (position + size > length) {
				std::cerr << "Unexpected EOF\n";
				return false;
			}
			
			// just copy contents straight into the struct, why the fuck not
			memcpy((void*)&t, buffer + position, size);

			// move buffer pointer forwards
			position += size;
			return true;
		}
	public:
		BSP bsp;

		Parser(std::string filename, bool verbose) {
			std::ifstream file;
			file.open(filename, std::ios::binary | std::ios::ate);

			this->verbose = verbose;

			if (!file.good()) {
				// File cannot be found or is used by something else
				std::cerr << "Cannot open file '" << filename << "'\n";
			}
			else {
				// Find file size
				std::streamsize size = file.tellg();

				// Move back to start, size file.tellg() moved the internal pointer
				file.seekg(0, std::ios::beg);

				// Initialize buffer
				this->buffer = new char[size];

				this->position = 0;
				this->length = size;

				// Copy file into buffer
				file.read(this->buffer, size);

				this->ready = true;
			}
		}

		~Parser() {
			delete[] buffer;
		}

		bool IsReady() {
			return this->ready;
		}

		bool Parse() {
			#define c(m) if (!m) return false

			auto checkoffset = [&](size_t offset, size_t size) {
				return (offset + size) < this->length;
			};

			// HEADER, since the header is always the fixed size we can just type-pun it
			verboseprint("Parsing header.. ");
			c(ReadType<dheader_t>(bsp.header));

			if (bsp.header.ident != VBSP_MAGIC_NUMBER) {
				std::cerr << "File is not of Source BSP format\n";
				return false;
			}

			verboseprint("BSP File version: " << bsp.header.version << "\t");
			verboseprint("Map revision: " << bsp.header.mapRevision << "\n");

			// LUMP TODO:
			/*
				0	DONE	10			30			40			50			60
				1	DONE	11			31			41			51			61
				2	DONE	12	DONE	32			42			52			62
				3	DONE	13	DONE	33			43			53			63
				4			14			34			44			54
				5			15			35			45			55
				6			16			36			46			56
				7	DONE	17			37			47			57
				8			18			38			48			58
				9			29			39			49			59
			*/

			// LUMP 0: Entities
			{
				verboseprint("Parsing lump 0 (entities)..\n");
				
				lump_t* lumpinfo = &bsp.header.lumps[0];

				if (!checkoffset(lumpinfo->fileofs, lumpinfo->filelen)) {
					std::cerr << "\nUnexpected EOF\n";
					return false;
				}

				bsp.lump0_entities.reserve(lumpinfo->filelen);
				memcpy((void*)bsp.lump0_entities.data(), this->buffer + lumpinfo->fileofs, lumpinfo->filelen);

				bsp.lumps[0] = (void*)&bsp.lump0_entities;
			}

			// LUMP 1: Planes
			{
				verboseprint("Parsing lump 1 (planes).. ");
				
				lump_t* lumpinfo = &bsp.header.lumps[1];

				if (!checkoffset(lumpinfo->fileofs, lumpinfo->filelen)) {
					std::cerr << "\nUnexpected EOF\n";
					return false;
				}
				
				size_t numPlanes = lumpinfo->filelen / sizeof(dplane_t);

				if (numPlanes > MAX_MAP_PLANES) {
					std::cerr << "BSP has more than the allowed amount of planes\n";
					return false;
				}

				for (int i = 0; i < numPlanes; i++) {
					dplane_t plane;
					memcpy((void*)&plane, this->buffer + lumpinfo->fileofs + i * sizeof(dplane_t), sizeof(dplane_t));
					bsp.lump1_planes.push_back(plane);
				}

				verboseprint(numPlanes << " planes\n");

				bsp.lumps[1] = (void*)&bsp.lump1_planes;
			}
			
			// LUMP 2: Texture data
			{
				verboseprint("Parsing lump 2 (texdata).. ");

				lump_t* lumpinfo = &bsp.header.lumps[2];

				if (!checkoffset(lumpinfo->fileofs, lumpinfo->filelen)) {
					std::cerr << "\nUnexpected EOF\n";
					return false;
				}

				size_t numTexdatas = lumpinfo->filelen / sizeof(dtexdata_t);

				if (numTexdatas > MAX_MAP_TEXDATAS) {
					std::cerr << "BSP has more than the allowed amount of texdatas\n";
					return false;
				}

				for (int i = 0; i < numTexdatas; i++) {
					dtexdata_t texdata;
					memcpy((void*)&texdata, this->buffer + lumpinfo->fileofs + i * sizeof(dtexdata_t), sizeof(dtexdata_t));
					bsp.lump2_texdata.push_back(texdata);
				}

				verboseprint(numTexdatas << " texdatas\n");

				bsp.lumps[2] = (void*)&bsp.lump2_texdata;
			}

			// LUMP 3: VERTEXES
			{
				verboseprint("Parsing lump 3 (vertexes).. ");

				lump_t* lumpinfo = &bsp.header.lumps[3];

				if (!checkoffset(lumpinfo->fileofs, lumpinfo->filelen)) {
					std::cerr << "\nUnexpected EOF\n";
					return false;
				}

				size_t numVerts = lumpinfo->filelen / sizeof(dvertex_t);

				if (numVerts > MAX_MAP_VERTS) {
					std::cerr << "BSP has more than the allowed amount of vertexes\n";
					return false;
				}

				for (int i = 0; i < numVerts; i++) {
					dvertex_t vert;
					memcpy((void*)&vert, this->buffer + lumpinfo->fileofs + i * sizeof(dvertex_t), sizeof(dvertex_t));
					bsp.lump3_vertexes.push_back(vert);
				}

				verboseprint(numVerts << " vertexes\n");

				bsp.lumps[3] = (void*)&bsp.lump3_vertexes;
			}

			// LUMP 7: FACE AND ORIGINAL FACE
			{
				verboseprint("Parsing lump 7 (faces).. ");

				lump_t* lumpinfo = &bsp.header.lumps[7];

				if (!checkoffset(lumpinfo->fileofs, lumpinfo->filelen)) {
					std::cerr << "\nUnexpected EOF\n";
					return false;
				}

				size_t numFaces = lumpinfo->filelen / sizeof(dface_t);

				if (numFaces > MAX_MAP_FACES) {
					std::cerr << "BSP has more than the allowed amount of faces\n";
					return false;
				}

				for (int i = 0; i < numFaces; i++) {
					dface_t face;
					memcpy((void*)&face, this->buffer + lumpinfo->fileofs + i * sizeof(dface_t), sizeof(dface_t));
					bsp.lump7_faces.push_back(face);
				}

				verboseprint(numFaces << " faces\n");

				bsp.lumps[7] = (void*)&bsp.lump7_faces;
			}

			// LUMP 12: EDGES
			{
				verboseprint("Parsing lump 12 (edges).. ");

				lump_t* lumpinfo = &bsp.header.lumps[12];

				if (!checkoffset(lumpinfo->fileofs, lumpinfo->filelen)) {
					std::cerr << "\nUnexpected EOF\n";
					return false;
				}

				size_t numEdges = lumpinfo->filelen / sizeof(dedge_t);

				if (numEdges > MAX_MAP_EDGES) {
					std::cerr << "BSP has more than the allowed amount of edges\n";
					return false;
				}

				for (int i = 0; i < numEdges; i++) {
					dedge_t edge;
					memcpy((void*)&edge, this->buffer + lumpinfo->fileofs + i * sizeof(dedge_t), sizeof(dedge_t));
					bsp.lump12_edges.push_back(edge);
				}

				verboseprint(numEdges << " edges\n");

				bsp.lumps[12] = (void*)&bsp.lump12_edges;
			}

			// LUMP 13: SURFEDGES
			{
				verboseprint("Parsing lump 13 (surfedges).. ");

				lump_t* lumpinfo = &bsp.header.lumps[13];

				if (!checkoffset(lumpinfo->fileofs, lumpinfo->filelen)) {
					std::cerr << "\nUnexpected EOF\n";
					return false;
				}

				size_t numSurfedges = lumpinfo->filelen / sizeof(dsurfedge_t);

				if (numSurfedges > MAX_MAP_SURFEDGES) {
					std::cerr << "BSP has more than the allowed amount of surfedges\n";
					return false;
				}

				for (int i = 0; i < numSurfedges; i++) {
					dsurfedge_t surfedge;
					memcpy((void*)&surfedge, this->buffer + lumpinfo->fileofs + i * sizeof(dsurfedge_t), sizeof(dsurfedge_t));
					bsp.lump13_surfedges.push_back(surfedge);
				}

				verboseprint(numSurfedges << " surfedges\n");

				bsp.lumps[13] = (void*)&bsp.lump13_surfedges;
			}

			return true;
			#undef c
		}
		
		#undef verboseprint
	};
	
}
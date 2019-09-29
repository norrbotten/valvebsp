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

			// HEADER, since the header is always the fixed size we can just type-pun it
			verboseprint("Parsing header.. ");
			c(ReadType<dheader_t>(bsp.header));

			if (bsp.header.ident != VBSP_MAGIC_NUMBER) {
				std::cerr << "File is not of Source BSP format\n";
				return false;
			}

			verboseprint("BSP File version: " << bsp.header.version << "\t");
			verboseprint("Map revision: " << bsp.header.mapRevision << "\n");

			// LUMP 0: Entities
			{
				verboseprint("Parsing lump 0 (entities)..\n");

				lump_t* lumpinfo = &bsp.header.lumps[0];

				bsp.lump0_entities.reserve(lumpinfo->filelen);
				memcpy(bsp.lump0_entities.data(), this->buffer + lumpinfo->fileofs, lumpinfo->filelen);

				bsp.lumps[0] = (void*)&bsp.lump0_entities;
			}

			// LUMP 1: Planes
			{
				verboseprint("Parsing lump 1 (planes).. ");
				
				lump_t* lumpinfo = &bsp.header.lumps[1];
				size_t numPlanes = lumpinfo->filelen / sizeof(dplane_t);

				if (numPlanes < MAX_MAP_PLANES) {
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

			// LUMP 3: VERTEXES
			{
				verboseprint("Parsing lump 3 (vertexes).. ");

				lump_t* lumpinfo = &bsp.header.lumps[3];
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

				bsp.lumps[3] = (void*)& bsp.lump3_vertexes;
			}

			return true;
			#undef c
		}
	};

	#undef verboseprint

}
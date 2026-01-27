//
// Created by Candy on 1/4/2026.
//  ObjLoader: reads .obj file -> converts it into your Mesh format (x y z u v + indices)
//

#include "ObjLoader.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"

//Static Helper Functions//

static int ResolveObjIndex(int idx , int listSize)
{
    //OBJ indices are 1-based.
    //Also if idx is negative it means "count from the end" (OBJ weirdness).
    //ie. if idx == 0 THOU ARE 'INVALIDDDDDDDDDDDDDD ! ' BEGONE SATAN

    if(idx > 0)
    {
        return idx - 1; //convert to 0-based
    }
    if (idx < 0)
    {
        return listSize + idx; //relative to end
    }

    return -1;
}

static bool ParseFaceToken(const std::string& tok, int& vIdx, int& vtIdx, int& vnIdx)
{
    // Face tokens can be:
    //  "v"
    //  "v/vt"
    //  "v//vn"
    //  "v/vt/vn"
    // We parse all of them so later we can support any OBJ file.

    vIdx = 0;
    vtIdx = 0;
    vnIdx = 0;

    size_t s1 = tok.find('/');
    if (s1 == std::string::npos)
    {
        // just "v"
        vIdx = std::stoi(tok);
        return true;
    }

    size_t s2 = tok.find('/', s1 + 1);

    // first chunk is always v
    vIdx = std::stoi(tok.substr(0, s1));

    if (s2 == std::string::npos)
    {
        // "v/vt"
        std::string a = tok.substr(s1 + 1);
        if (!a.empty())
            vtIdx = std::stoi(a);
        return true;
    }

    if (s2 == s1 + 1)
    {
        // "v//vn" (empty vt)
        std::string c = tok.substr(s2 + 1);
        if (!c.empty())
            vnIdx = std::stoi(c);
        return true;
    }

    // "v/vt/vn"
    std::string b = tok.substr(s1 + 1, s2 - (s1 + 1));
    std::string c = tok.substr(s2 + 1);

    if (!b.empty()) vtIdx = std::stoi(b);
    if (!c.empty()) vnIdx = std::stoi(c);

    return true;
}

// This is our "unique vertex identity" for dedup.
// Because OBJ indexes position/uv/normal separately, but GPU wants ONE packed vertex.
// So we map (v,vt,vn) -> final vertex index.
struct VertexKey
{
    int v;
    int vt;
    int vn;

    bool operator==(const VertexKey& other) const
    {
        return v == other.v && vt == other.vt && vn == other.vn;
    }
};

struct VertexKeyHash
{
    size_t operator()(const VertexKey& k) const noexcept
    {
        // cheap-ish hash combine
        size_t h1 = std::hash<int>{}(k.v);
        size_t h2 = std::hash<int>{}(k.vt);
        size_t h3 = std::hash<int>{}(k.vn);
        return (h1 * 73856093u) ^ (h2 * 19349663u) ^ (h3 * 83492791u);
    }
};

ObjMeshData LoadOBJ(const std::string &path, bool flipV)
{
    ObjMeshData Data;

    std::ifstream objFile(path);
    if(!objFile.is_open())
    {
        std::cerr<<"[OBJLoader] Failed to open OBJ: "<< path <<std::endl;
        return Data;
    }

    int vCount = 0;
    int vnCount = 0;
    int vtCount = 0;
    int fCount = 0;

    //To store vectors and UV's
    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;

    // Absolutely no clue what I did here ......and i forgot to doc it!
    //I mean I do know what I did its just reading a file a and finding
    //the right count for the vertices or faces etc and adding their values still not very clear

    // This is the "magic engine part":
    // map (v,vt,vn) -> packed vertex index (so we don't duplicate vertices constantly)
    std::unordered_map<VertexKey, unsigned int, VertexKeyHash> vertexMap;
    vertexMap.reserve(10000);

    std::string line;

    while(std::getline(objFile , line))
    {
        if (line.empty() || line[0] == '#')
        {
            continue;
        }

        //1 . positons - x , y , z//
        if(line.rfind("v " , 0) == 0)
        {
            vCount++;
            std::stringstream ss(line);
            std::string prefix;
            glm::vec3 p;
            ss>>prefix>>p.x>>p.y>>p.z;
            positions.push_back(p);
        }

        //2 . vt - i.e u and v for da textures although idont think the obj I imported had uv..hmm//
        else if(line.rfind("vt " , 0) == 0)
        {
            vtCount++;
            Data.hasUVs = true;

            std::stringstream ss(line);
            std::string prefix;
            glm::vec2 t;//TODO - Need to ask how to properly name these temp variables inside of scope
            ss>>prefix>>t.x>>t.y;

            //flip V if textures show upside down later (common annoying thing)
            if (flipV)
                t.y = 1.0f - t.y;

            uvs.push_back(t);
        }

        //Normals if we do want to count and store them
        else if(line.rfind("vn " , 0) == 0)
        {
            vnCount++;
            Data.hasNormals = true;

            std::stringstream ss(line);
            std::string prefix;
            glm::vec3 n;
            ss >> prefix >> n.x >> n.y >> n.z;
            normals.push_back(n);
        }

        //Faces i.e f's  ffffffffff : L
        else if(line.rfind("f " , 0) == 0)
        {
            fCount++;

            // Parse face line tokens
            std::stringstream ss(line);
            std::string prefix;
            ss >> prefix;

            std::vector<unsigned int> faceVertexIndices;
            std::string tok;

            // Read tokens like: "12/3/7"
            while (ss >> tok)
            {
                int vI = 0, vtI = 0, vnI = 0;
                if (!ParseFaceToken(tok, vI, vtI, vnI))
                    continue;

                int v0  = ResolveObjIndex(vI, (int)positions.size());
                int vt0 = (vtI != 0) ? ResolveObjIndex(vtI, (int)uvs.size()) : -1;
                int vn0 = (vnI != 0) ? ResolveObjIndex(vnI, (int)normals.size()) : -1;

                if (v0 < 0 || v0 >= (int)positions.size())
                    continue;

                VertexKey key{ v0, vt0, vn0 };

                auto it = vertexMap.find(key);
                if (it == vertexMap.end())
                {
                    // Upgraded to // Create a new packed vertex (x y z u v nx ny nz)

                    const glm::vec3& p = positions[v0];

                    glm::vec2 uv(0.0f, 0.0f);
                    if (vt0 >= 0 && vt0 < (int)uvs.size())
                        uv = uvs[vt0]; // if OBJ has UVs we use them, if not its (0,0)

                    glm::vec3 n(0.0f, 0.0f, 1.0f);
                    if (vn0 >= 0 && vn0 < (int)normals.size())
                        n = normals[vn0];

                    unsigned int newIndex = (unsigned int)(Data.vertices.size() / 8);

                    Data.vertices.push_back(p.x);
                    Data.vertices.push_back(p.y);
                    Data.vertices.push_back(p.z);
                    Data.vertices.push_back(uv.x);
                    Data.vertices.push_back(uv.y);
                    Data.vertices.push_back(n.x);
                    Data.vertices.push_back(n.y);
                    Data.vertices.push_back(n.z);


                    vertexMap.emplace(key, newIndex);
                    faceVertexIndices.push_back(newIndex);
                }
                else
                {
                    // Already made this exact vertex combo before
                    faceVertexIndices.push_back(it->second);
                }
            }

            // Triangulate fan style (works for triangles/quads/ngons)
            // (0,1,2) (0,2,3) (0,3,4) ...
            for (size_t i = 1; i + 1 < faceVertexIndices.size(); ++i)
            {
                Data.indices.push_back(faceVertexIndices[0]);
                Data.indices.push_back(faceVertexIndices[i]);
                Data.indices.push_back(faceVertexIndices[i + 1]);
            }
        }
    }

    //TODO -- Remember to delete this please here to just test this HO~ //
    std::cout << "[ObjLoader] " << path
              << " | v=" << vCount
              << " vt=" << vtCount
              << " vn=" << vnCount
              << " f=" << fCount
              << " | packedVerts=" << (Data.vertices.size() / 5)
              << " tris=" << (Data.indices.size() / 3)
              << "\n";

    if (!positions.empty())
    {
        const auto& p = positions[0];
        std::cout << "  first v: (" << p.x << ", " << p.y << ", " << p.z << ")\n";
    }
    if (!uvs.empty())
    {
        const auto& t = uvs[0];
        std::cout << "  first vt: (" << t.x << ", " << t.y << ")\n";
    }
    if (!Data.vertices.empty())
    {
        std::cout << "  first packed vertex: ("
                  << Data.vertices[0] << ", " << Data.vertices[1] << ", " << Data.vertices[2]
                  << ") uv=(" << Data.vertices[3] << ", " << Data.vertices[4] << ")\n";
    }

    return Data;
}

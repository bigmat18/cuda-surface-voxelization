#include <voxelization/voxelization.h>
#include <bounding_box.h>

template <typename T>
 __host__ void Sequential(VoxelsGrid<T, false> &grid,
                          const std::vector<uint32_t>& triangleCoords,
                          const std::vector<Position>& coords)
{
    const int numTriangle = triangleCoords.size() / 3;
    for(int i = 0; i < numTriangle; ++i) {
        Position V0 = coords[triangleCoords[(i * 3)]];
        Position V1 = coords[triangleCoords[(i * 3) + 1]];
        Position V2 = coords[triangleCoords[(i * 3) + 2]];
            
        auto [A0, B0, C0] = CalculateEdgeTerms(V0, V1);
        auto [A1, B1, C1] = CalculateEdgeTerms(V1, V2);
        auto [A2, B2, C2] = CalculateEdgeTerms(V2, V0);

        Position facesVertices[3] = {V0, V1, V2};
        std::pair<float, float> BB_X, BB_Y, BB_Z;
        CalculateBoundingBox(std::span<Position>(&facesVertices[0], 3), BB_X, BB_Y, BB_Z);

        int startY = static_cast<int>(std::floorf((BB_Y.first - grid.OriginY()) / grid.VoxelSize()));
        int endY   = static_cast<int>(std::ceilf((BB_Y.second - grid.OriginY()) / grid.VoxelSize()));
        int startZ = static_cast<int>(std::floorf((BB_Z.first - grid.OriginZ()) / grid.VoxelSize()));
        int endZ   = static_cast<int>(std::ceilf((BB_Z.second - grid.OriginZ()) / grid.VoxelSize()));

        Position edge0 = V1 - V0;
        Position edge1 = V2 - V0;
        auto [A, B, C] = Position::Cross(edge0, edge1);
        float D = Position::Dot({A, B, C}, V0);

        for(int y = startY; y < endY; ++y)
        {
            for(int z = startZ; z < endZ; ++z)
            {
                float centerY = grid.OriginY() + ((y * grid.VoxelSize()) + (grid.VoxelSize() / 2));
                float centerZ = grid.OriginZ() + ((z * grid.VoxelSize()) + (grid.VoxelSize() / 2));

                float E0 = CalculateEdgeFunction(A0, B0, C0, centerY, centerZ);
                float E1 = CalculateEdgeFunction(A1, B1, C1, centerY, centerZ);
                float E2 = CalculateEdgeFunction(A2, B2, C2, centerY, centerZ);

                bool ccw_test = (E0 >= 0 && E1 >= 0 && E2 >= 0);
                bool cw_test = (E0 <= 0 && E1 <= 0 && E2 <= 0);
                
                if (ccw_test || cw_test) {
                    float intersection = (D - (B * centerY) - (C * centerZ)) / A;

                    int startX = static_cast<int>((intersection - grid.OriginX()) / grid.VoxelSize());
                    int endX = grid.VoxelsPerSide();
                    for(int x = startX; x < endX; ++x)
                        grid(x, y, z) ^= true;
                }
            }
        }
    }
}

template __host__ void Sequential<uint8_t>
    (VoxelsGrid<uint8_t, false>&, const std::vector<uint32_t>&, const std::vector<Position>&);

template __host__ void Sequential<uint16_t>
    (VoxelsGrid<uint16_t, false>&, const std::vector<uint32_t>&, const std::vector<Position>&);
    
template __host__ void Sequential<uint32_t>
    (VoxelsGrid<uint32_t, false>&, const std::vector<uint32_t>&, const std::vector<Position>&);

template __host__ void Sequential<uint64_t>
    (VoxelsGrid<uint64_t, false>&, const std::vector<uint32_t>&, const std::vector<Position>&);

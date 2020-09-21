#include "Physics_TD.h"

inline physics_infos
InitializePhysics()
{
    physics_infos Result = {true, {0, -9.81f, 0}, 0.01f, 1};
    
    return Result;
}

inline b32 
AABBIntersectionTest(aa_bounding_box A, aa_bounding_box B)
{
    b32 Result = ((A.Min.x <= B.Max.x && A.Max.x >= B.Min.x) &&
                  (A.Min.y <= B.Max.y && A.Max.y >= B.Min.y) &&
                  (A.Min.z <= B.Max.z && A.Max.z >= B.Min.z) );
    return Result;
}

internal collision_box_list
CollectPotentialCollisions(collision_box_list ColBoxesList, collision_box *Collider)
{
    collision_box_list Result = {};
    aa_bounding_box AABB = Collider->AABB;
    
    For(ColBoxesList.Count)
    {
        if(AABBIntersectionTest(AABB, ColBoxesList.Boxes[It]->AABB))
        {
            Result.Boxes[Result.Count++] = ColBoxesList.Boxes[It];
        }
    }
    
    return Result;
}

internal void
UpdateAABB(collision_box *CBox)
{
    aa_bounding_box Result = {V3(MAX_REAL32), V3(MIN_REAL32)};
    
    For(8)
    {
        v3 Corner = CBox->Center + CBox->Orientation*CBox->Corners[It];
        Result.Min.x = Min(Result.Min.x, Corner.x);
        Result.Min.y = Min(Result.Min.y, Corner.y);
        Result.Min.z = Min(Result.Min.z, Corner.z);
        
        Result.Max.x = Max(Result.Max.x, Corner.x);
        Result.Max.y = Max(Result.Max.y, Corner.y);
        Result.Max.z = Max(Result.Max.z, Corner.z);
    }
    
    CBox->AABB = Result;
}

inline void
UpdateCollider(collision_box *Collider, v3 *Position, m4 *Rotation)
{
    Collider->Center = *Position;
    Collider->Orientation = *Rotation;
    UpdateAABB(Collider);
}

inline collision_box
CreateCollisionBox(physics_infos *Physics, b32 IsStatic, v3 Center, v3 Extends, m4 LocalInertiaTensor, r32 Bounciness, m4 Orientation = Identity())
{
    collision_box Result = {0};
    
    Result.ID = Physics->CollisionBoxCount++;
    Result.IsStatic    = IsStatic;
    Result.Center      = Center;
    Result.Orientation = Orientation;
    Result.Extends     = Extends;
    Result.Bounciness  = Bounciness;
    Result.LocalInertiaTensor = LocalInertiaTensor;
    
    Result.Corners[0] = V3( Extends.x, Extends.y, Extends.z);
    Result.Corners[1] = V3( Extends.x, Extends.y,-Extends.z);
    Result.Corners[2] = V3( Extends.x,-Extends.y,-Extends.z);
    Result.Corners[3] = V3(-Extends.x,-Extends.y,-Extends.z);
    Result.Corners[4] = V3(-Extends.x,-Extends.y, Extends.z);
    Result.Corners[5] = V3(-Extends.x, Extends.y, Extends.z);
    Result.Corners[6] = V3( Extends.x,-Extends.y, Extends.z);
    Result.Corners[7] = V3(-Extends.x, Extends.y,-Extends.z);
    
    UpdateAABB(&Result);
    
    return Result;
}

internal v3
GJK_Support(v3 Direction, collision_box *Collider)
{
    v3 Result = {};
    r32 Dist = MIN_REAL32;
    
    For(8)
    {
        v3 TestCorner = Collider->Orientation*Collider->Corners[It];
        r32 NewDist = Dot(Direction, TestCorner);
        if(NewDist > Dist)
        {
            Dist = NewDist;
            Result = TestCorner;
        }
    }
    
    Result += Collider->Center;
    return Result;
}

inline v3
GJK_SupportInDifference(v3 Direction, collision_box *Shape1, collision_box *Shape2)
{
    v3 Result = {};
    
    Result = GJK_Support(Direction, Shape1) - GJK_Support(-Direction, Shape2);
    
    return Result;
}

struct pers_epa
{
    face Faces[EPA_MAX_FACES];
    u32 FaceCount;
    u32 ClosestFace;
    
    b32 IsDone;
};

inline pers_epa
InitializeEPA_Incremental(simplex Simplex)
{
    pers_epa Result = {};
    
    v3 AB = Simplex.B-Simplex.A;
    v3 AC = Simplex.C-Simplex.A;
    v3 AD = Simplex.D-Simplex.A;
    v3 BD = Simplex.D-Simplex.B;
    v3 BC = Simplex.C-Simplex.B;
    
    Result.FaceCount = 4;
    
    // Creating CCW faces 
    Result.Faces[0].Points[0] = Simplex.A;
    Result.Faces[0].Points[1] = Simplex.B;
    Result.Faces[0].Points[2] = Simplex.C;
    Result.Faces[0].N         = Normalize(Cross(AB, AC));
    Result.Faces[1].Points[0] = Simplex.A;
    Result.Faces[1].Points[1] = Simplex.C;
    Result.Faces[1].Points[2] = Simplex.D;
    Result.Faces[1].N         = Normalize(Cross(AC, AD));
    Result.Faces[2].Points[0] = Simplex.A;
    Result.Faces[2].Points[1] = Simplex.D;
    Result.Faces[2].Points[2] = Simplex.B;
    Result.Faces[2].N         = Normalize(Cross(AD, AB));
    Result.Faces[3].Points[0] = Simplex.B;
    Result.Faces[3].Points[1] = Simplex.D;
    Result.Faces[3].Points[2] = Simplex.C;
    Result.Faces[3].N         = Normalize(Cross(BD, BC));
    
    return Result;
}

internal b32
EPA_Incremental(simplex Simplex, collision_box *CBoxA, collision_box *CBoxB, pers_epa *PersistData)
{
    b32 Result = false;
    
    For(EPA_MAX_ITERATIONS)
    {
        // Find face thats closest to origin
        r32 MinDist = Dot(PersistData->Faces[0].A, PersistData->Faces[0].N);
        PersistData->ClosestFace = 0;
        for(u32 FID = 1; FID < PersistData->FaceCount; ++FID)
        {
            r32 Dist = Abs(Dot(PersistData->Faces[FID].A, PersistData->Faces[FID].N));
            if(Dist < MinDist)
            {
                MinDist = Dist;
                PersistData->ClosestFace = FID;
            }
        }
        
        // Origin lies directly on face
        if(MinDist == 0.0f)
        {
            printf("Min dist was 0.0!\n");
            Result = true;
            break;
        }
        
        // Search normal to face thats closest to origin
        v3 SearchDir = PersistData->Faces[PersistData->ClosestFace].N;
        v3 P2 = GJK_Support(SearchDir, CBoxB) - GJK_Support(-SearchDir, CBoxA);
        v3 P = GJK_SupportInDifference(SearchDir, CBoxA, CBoxB);
        r32 Depth = Dot(P, SearchDir);
        if(Depth - MinDist < EPA_TOLERANCE) 
        {
            Result = true;
            break;
        }
        
        edge LooseEdges[EPA_MAX_EDGES];
        u32 LooseEdgeCount = 0;
        
        // Find all triangles that are facing P
        for(u32 FID = 0; FID < PersistData->FaceCount; ++FID)
        {
            if(Dot(PersistData->Faces[FID].N, P-PersistData->Faces[FID].A) > 0.0f)
            {
                for(u32 PID = 0; PID < 3; ++PID)
                {
                    edge CurrentEdge = 
                    {
                        PersistData->Faces[FID].Points[PID], 
                        PersistData->Faces[FID].Points[(PID+1)%3]
                    };
                    b32 FoundEdge = false;
                    
                    for(u32 EID = 0; EID < LooseEdgeCount; ++EID)
                    {
                        if(LooseEdges[EID].B == CurrentEdge.A &&
                           LooseEdges[EID].A == CurrentEdge.B)
                        {
                            LooseEdges[EID].A = LooseEdges[LooseEdgeCount-1].A;
                            LooseEdges[EID].B = LooseEdges[LooseEdgeCount-1].B;
                            LooseEdgeCount--;
                            FoundEdge = true;
                            break;
                        }
                    }
                    
                    if(!FoundEdge)
                    {
                        Assert(LooseEdgeCount < EPA_MAX_EDGES);
                        LooseEdges[LooseEdgeCount].A = CurrentEdge.A;
                        LooseEdges[LooseEdgeCount].B = CurrentEdge.B;
                        LooseEdgeCount++;
                    }
                }
                
                PersistData->Faces[FID].A = PersistData->Faces[PersistData->FaceCount-1].A;
                PersistData->Faces[FID].B = PersistData->Faces[PersistData->FaceCount-1].B;
                PersistData->Faces[FID].C = PersistData->Faces[PersistData->FaceCount-1].C;
                PersistData->Faces[FID].N = PersistData->Faces[PersistData->FaceCount-1].N;
                PersistData->FaceCount--;
                FID--;
            }
        }
        
        // Reconstruct polytope with P added
        for(u32 EID = 0; EID < LooseEdgeCount; ++EID)
        {
            // Assert(FaceCount < EPA_MAX_FACES);
            if(PersistData->FaceCount >= EPA_MAX_FACES) break;
            PersistData->Faces[PersistData->FaceCount].A = LooseEdges[EID].A;
            PersistData->Faces[PersistData->FaceCount].B = LooseEdges[EID].B;
            PersistData->Faces[PersistData->FaceCount].C = P;
            v3 N = Cross(LooseEdges[EID].A-LooseEdges[EID].B, LooseEdges[EID].A-P);
            if(LengthSquared(N) == 0.0f) 
            {
                printf("***********N has ZERO length!***********\n");
            }
            else ;
            PersistData->Faces[PersistData->FaceCount].N = Normalize(N);
            
            r32 Bias = 0.000001f;
            if(Dot(PersistData->Faces[PersistData->FaceCount].A, PersistData->Faces[PersistData->FaceCount].N) + Bias < 0)
            {
                v3 Tmp = PersistData->Faces[PersistData->FaceCount].A;
                PersistData->Faces[PersistData->FaceCount].A = PersistData->Faces[PersistData->FaceCount].B;
                PersistData->Faces[PersistData->FaceCount].B = Tmp;
                PersistData->Faces[PersistData->FaceCount].N = -PersistData->Faces[PersistData->FaceCount].N;
            }
            PersistData->FaceCount++;
        }
    }
    
    return Result;
}

internal v3 
EPA(simplex Simplex, collision_box *CBoxA, collision_box *CBoxB)
{
    v3 Result = V3(MIN_REAL32);
    
    v3 AB = Simplex.B-Simplex.A;
    v3 AC = Simplex.C-Simplex.A;
    v3 AD = Simplex.D-Simplex.A;
    v3 BD = Simplex.D-Simplex.B;
    v3 BC = Simplex.C-Simplex.B;
    
    face Faces[EPA_MAX_FACES];
    u32 FaceCount = 4;
    u32 ClosestFace = 0;
    
    
    // Creating CCW faces 
    Faces[0].Points[0] = Simplex.A;
    Faces[0].Points[1] = Simplex.B;
    Faces[0].Points[2] = Simplex.C;
    Faces[0].N         = Normalize(Cross(AB, AC));
    Faces[1].Points[0] = Simplex.A;
    Faces[1].Points[1] = Simplex.C;
    Faces[1].Points[2] = Simplex.D;
    Faces[1].N         = Normalize(Cross(AC, AD));
    Faces[2].Points[0] = Simplex.A;
    Faces[2].Points[1] = Simplex.D;
    Faces[2].Points[2] = Simplex.B;
    Faces[2].N         = Normalize(Cross(AD, AB));
    Faces[3].Points[0] = Simplex.B;
    Faces[3].Points[1] = Simplex.D;
    Faces[3].Points[2] = Simplex.C;
    Faces[3].N         = Normalize(Cross(BD, BC));
    
    b32 POnEdge = false;
    u32 TC = 0;
    For(EPA_MAX_ITERATIONS)
    {
        // Find face thats closest to origin
        r32 MinDist = Dot(Faces[0].A, Faces[0].N);
        ClosestFace = 0;
        for(u32 FID = 1; FID < FaceCount; ++FID)
        {
            r32 Dist = Dot(Faces[FID].A, Faces[FID].N);
            if(Dist < MinDist)
            {
                MinDist = Dist;
                ClosestFace = FID;
            }
        }
        
        // Origin lies directly on face
        if(MinDist == 0.0f)
        {
            printf("Min dist was 0.0!\n");
            Result = Faces[ClosestFace].N; // Depth would be 0?
            break;
        }
        
        // Search normal to face thats closest to origin
        v3 SearchDir = Faces[ClosestFace].N;
        v3 P = GJK_SupportInDifference(SearchDir, CBoxA, CBoxB);
        r32 Depth = Dot(P, SearchDir);
        if(Depth - MinDist < EPA_TOLERANCE) 
        {
            Result = Faces[ClosestFace].N*Depth;
            break;
        }
        
        edge LooseEdges[EPA_MAX_EDGES];
        u32 LooseEdgeCount = 0;
        
        // Find all triangles that are facing P
        for(u32 FID = 0; FID < FaceCount; ++FID)
        {
            if(Dot(Faces[FID].N, P-Faces[FID].A) > 0.0f)
            {
                for(u32 PID = 0; PID < 3; ++PID)
                {
                    edge CurrentEdge = 
                    {
                        Faces[FID].Points[PID], 
                        Faces[FID].Points[(PID+1)%3]
                    };
                    b32 FoundEdge = false;
                    
                    for(u32 EID = 0; EID < LooseEdgeCount; ++EID)
                    {
                        if(LooseEdges[EID].B == CurrentEdge.A &&
                           LooseEdges[EID].A == CurrentEdge.B)
                        {
                            LooseEdges[EID].A = LooseEdges[LooseEdgeCount-1].A;
                            LooseEdges[EID].B = LooseEdges[LooseEdgeCount-1].B;
                            LooseEdgeCount--;
                            FoundEdge = true;
                            break;
                        }
                    }
                    
                    if(!FoundEdge)
                    {
                        Assert(LooseEdgeCount < EPA_MAX_EDGES);
                        LooseEdges[LooseEdgeCount].A = CurrentEdge.A;
                        LooseEdges[LooseEdgeCount].B = CurrentEdge.B;
                        LooseEdgeCount++;
                    }
                }
                
                Faces[FID].A = Faces[FaceCount-1].A;
                Faces[FID].B = Faces[FaceCount-1].B;
                Faces[FID].C = Faces[FaceCount-1].C;
                Faces[FID].N = Faces[FaceCount-1].N;
                FaceCount--;
                FID--;
            }
        }
        
        // Reconstruct polytope with P added
        for(u32 EID = 0; EID < LooseEdgeCount; ++EID)
        {
            Assert(FaceCount < EPA_MAX_FACES);
            //if(FaceCount >= EPA_MAX_FACES) break;
            Faces[FaceCount].A = LooseEdges[EID].A;
            Faces[FaceCount].B = LooseEdges[EID].B;
            Faces[FaceCount].C = P;
            v3 N = Cross(LooseEdges[EID].A-LooseEdges[EID].B, LooseEdges[EID].A-P);
            if(LengthSquared(N) == 0.0f) 
            {
                printf("***********N has ZERO length!***********\n");
                POnEdge = true;
                break;
            }
            else ;
            Faces[FaceCount].N = Normalize(N);
            
            r32 Bias = 0.000001f;
            if(Dot(Faces[FaceCount].A, Faces[FaceCount].N) + Bias < 0)
            {
                v3 Tmp = Faces[FaceCount].A;
                Faces[FaceCount].A = Faces[FaceCount].B;
                Faces[FaceCount].B = Tmp;
                Faces[FaceCount].N = -Faces[FaceCount].N;
            }
            FaceCount++;
        }
        if(POnEdge) break;
        TC++;
    }
    
    //printf("%d\n", TC);
    if(Result.x == MIN_REAL32) 
    {
        printf("ERROR:: EPA did not converge!\n");
        Result = Faces[ClosestFace].N*Dot(Faces[ClosestFace].A, Faces[ClosestFace].N);
    }
    
    return Result;
}

internal void
GJK_Triangle(simplex *Simplex, v3 *Dir)
{
    v3 AO  = -Simplex->A;
    v3 AB  = Simplex->B - Simplex->A;
    v3 AC  = Simplex->C - Simplex->A;
    v3 ABC = Cross(AB, AC);
    
    v3 ABN = Cross(AB, ABC);
    if (Dot(ABN, AO) > 0.0f) // Closest to edge AB
    {
        Simplex->C = Simplex->A;
        Simplex->Count = 2;
        *Dir = TripleCross(AB, AO, AB);
    }
    else
    {
        v3 ACN = Cross(ABC, AC); 
        if (Dot(ACN, AO) > 0.0f) // Closest to edge AC
        {
            Simplex->B = Simplex->A;
            Simplex->Count = 2;
            *Dir = TripleCross(AC, AO, AC);
        }
        else
        {
            if (Dot(ABC, AO) > 0.0f)
            {
                Simplex->D = Simplex->C;
                Simplex->C= Simplex->B;
                Simplex->B = Simplex->A;
                *Dir = ABC;
            }
            else
            {
                Simplex->D = Simplex->B;
                Simplex->B = Simplex->A;
                *Dir = -ABC;
            }
            Simplex->Count = 3;
        }
    }
}

internal void
GJK_CheckTetrahedron(simplex *Simplex, v3 *AO, v3 *AB, v3 *AC, v3 *ABC, v3 *Dir)
{
    v3 AB_ABC = Cross(*AB, *ABC);
    if (Dot(AB_ABC, *AO) > 0.0f)
    {
        Simplex->C = Simplex->B;
        Simplex->B = Simplex->A;
        *Dir = TripleCross(*AB, *AO, *AB);
        Simplex->Count = 2;
    }
    else
    {
        v3 ACP = Cross(*ABC, *AC);
        if (Dot(ACP, *AO) > 0.0f)
        {
            Simplex->B = Simplex->A;
            *Dir = TripleCross(*AC, *AO, *AC);
            Simplex->Count = 2;
        }
        else
        {
            Simplex->D = Simplex->C;
            Simplex->C = Simplex->B;
            Simplex->B = Simplex->A;
            *Dir = *ABC;
            Simplex->Count = 3;
        }
    }
}

internal b32
GJK_Tetrahedron(simplex *Simplex, v3 *Dir)
{
    b32 Result = false;
    v3 AO  = -Simplex->A;
    v3 AB  = Simplex->B - Simplex->A;
    v3 AC  = Simplex->C - Simplex->A;
    v3 ABC = Cross(AB, AC);
    Simplex->Count = 3;
    
    if (Dot(ABC, AO) > 0.0f) 
    {
        GJK_CheckTetrahedron(Simplex, &AO, &AB, &AC, &ABC, Dir);
    }
    else
    {
        v3 AD  = Simplex->D - Simplex->A;
        v3 ACD = Cross(AC, AD);
        
        if (Dot(ACD, AO) > 0.0f)
        {
            Simplex->B   = Simplex->C;
            Simplex->C   = Simplex->D;
            AB  = AC;
            AC  = AD;
            ABC = ACD;
            
            GJK_CheckTetrahedron(Simplex, &AO, &AB, &AC, &ABC, Dir);
        }
        else
        {
            v3 ADB = Cross(AD, AB);
            
            if (Dot(ADB, AO) > 0.0f)
            {
                Simplex->C   = Simplex->B;
                Simplex->B   = Simplex->D;
                AC  = AB;
                AB  = AD;
                ABC = ADB;
                
                GJK_CheckTetrahedron(Simplex, &AO, &AB, &AC, &ABC, Dir);
            }
            else Result = true;
        }
    }
    return Result;
}

inline void
GJK_ExpandSimplex(simplex *Simplex, collision_box *CBoxA, collision_box *CBoxB)
{
    v3 AB = Simplex->B - Simplex->A;
    v3 AC = Simplex->C - Simplex->A;
    v3 N = Cross(AB, AC);
    Simplex->D = GJK_SupportInDifference(N, CBoxA, CBoxB);
    if(LengthSquared(Simplex->D) < 0.00001f)
    {
        Simplex->D = GJK_SupportInDifference(-N, CBoxA, CBoxB);
    }
    Simplex->Count++;
}

internal simplex
GJK_ForIncrementalEPA(collision_box *CBoxA, collision_box *CBoxB, v3 *Penetration)
{
    simplex Simplex = {};
    
    v3 Direction = Normalize(CBoxA->Center-CBoxB->Center);
    v3 Origin = {};
    
    Simplex.C  = GJK_SupportInDifference(Direction, CBoxA, CBoxB);
    Direction = -Direction;
    Simplex.B  = GJK_SupportInDifference(Direction, CBoxA, CBoxB);
    Simplex.Count = 2;
    
    if(Dot(Simplex.B, Direction) >= 0.0f)
    {
        v3 BC = Simplex.C - Simplex.B;
        Direction = TripleCross(BC, -Simplex.B, BC);
        if(LengthSquared(Direction) == 0.0f) // Origin on line segment
        {
            Direction = Cross(BC, V3(1, 0, 0));
            if(LengthSquared(Direction) == 0.0f) Direction = Cross(BC, V3(0, 0, -1));
        }
        
        For(GJK_MAX_ITERATIONS)
        {
            Simplex.A = GJK_SupportInDifference(Direction, CBoxA, CBoxB);
            Simplex.Count++;
            if(Dot(Simplex.A, Direction) < 0.0f) break;
            else
            {
                if(Simplex.Count == 2)
                {
                    GJK_Triangle(&Simplex, &Direction);
                }
                else if(GJK_Tetrahedron(&Simplex, &Direction))
                {
                    if(It == 0) 
                    {
                        printf("Simplex only a triangle! %d\n", Simplex.Count);
                        GJK_ExpandSimplex(&Simplex, CBoxA, CBoxB);
                    }
                    break;
                }
            }
        }
    }
    return Simplex;
}

internal b32
GJK(collision_box *CBoxA, collision_box *CBoxB, v3 *Penetration)
{
    b32 Result = false;
    simplex Simplex = {};
    
    v3 Direction = Normalize(CBoxA->Center-CBoxB->Center);
    v3 Origin = {};
    
    Simplex.C  = GJK_SupportInDifference(Direction, CBoxA, CBoxB);
    Direction = -Direction;
    Simplex.B  = GJK_SupportInDifference(Direction, CBoxA, CBoxB);
    Simplex.Count = 2;
    
    if(Dot(Simplex.B, Direction) >= 0.0f)
    {
        v3 BC = Simplex.C - Simplex.B;
        Direction = TripleCross(BC, -Simplex.B, BC);
        if(LengthSquared(Direction) == 0.0f) // Origin on line segment
        {
            Direction = Cross(BC, V3(1, 0, 0));
            if(LengthSquared(Direction) == 0.0f) Direction = Cross(BC, V3(0, 0, -1));
        }
        
        For(GJK_MAX_ITERATIONS)
        {
            Simplex.A = GJK_SupportInDifference(Direction, CBoxA, CBoxB);
            Simplex.Count++;
            if(Dot(Simplex.A, Direction) < 0.0f) break;
            else
            {
                if(Simplex.Count == 2)
                {
                    GJK_Triangle(&Simplex, &Direction);
                }
                else if(GJK_Tetrahedron(&Simplex, &Direction))
                {
                    if(It == 0) 
                    {
                        GJK_ExpandSimplex(&Simplex, CBoxA, CBoxB);
                        printf("Simplex only a triangle! %d\n", Simplex.Count);
                    }
                    if(Penetration) *Penetration = EPA(Simplex, CBoxA, CBoxB);
                    Result = true;
                    break;
                }
            }
        }
    }
    return Result;
}

inline rigid_body
CreateRigidBody(physics_infos *Physics, r32 Mass, m4 LocalInertiaTensor, v3 LocalCentroid, v3 Position, r32 Scale, m4 Orientation)
{
    rigid_body Result = {};
    
    Result.Mass = Mass;
    Result.InverseMass = 1.0f/Mass;
    Result.Collider = CreateCollisionBox(Physics, false, Position, V3(Scale*0.5f), LocalInertiaTensor, 0.25f),
    Result.LocalCentroid = LocalCentroid;
    Result.Position = Position;
    Result.Orientation = Orientation;
    Result.InverseOrientation = Orientation;
    Transpose(&Result.InverseOrientation);
    
    UpdateGlobalCentroidFromPosition(&Result);
    UpdateInertiaTensor(&Result);
    return Result;
}

inline void
UpdateGlobalCentroidFromPosition(rigid_body *RB)
{
    RB->GlobalCentroid = RB->Orientation*RB->LocalCentroid+RB->Position;
}

inline void
UpdatePositionFromGlobalCentroid(rigid_body *RB)
{
    RB->Position = RB->Orientation*(-RB->LocalCentroid) + RB->GlobalCentroid;
}

inline v3 
LocalToGlobal(rigid_body *RB, v3 P)
{
    v3 Result = RB->Orientation*P + RB->Position;
    return Result;
}

inline v3 
GlobalToLocal(rigid_body *RB, v3 P)
{
    v3 Result = RB->InverseOrientation*(P - RB->Position);
    return Result;
}

inline v3 
LocalToGlobalVector(rigid_body *RB, v3 V)
{
    v3 Result = RB->Orientation*V;
    return Result;
}

inline v3 
GlobalToLocalVector(rigid_body *RB, v3 V)
{
    v3 Result = RB->InverseOrientation*V;
    return Result;
}

inline void
ApplyForce(rigid_body *RB, v3 Force, v3 ForceAt)
{
    RB->ForceAccumulator  += Force;
    RB->TorqueAccumulator += Cross((ForceAt - RB->GlobalCentroid), Force);
}

inline void
UpdateOrientation(rigid_body *RB)
{
    // TODO(TD): Find reason for jitter when using first method. Second method
    // seems to be a dead-end.
#if 0
    quaternion Q = MatrixToQuaternion(RB->Orientation);
    Normalize(&Q);
    m4 Orientation  = QuaternionToMatrix(Q);
    RB->Orientation = Orientation;
#elif 0
    quaternion Q = MatrixToQuaternion(RB->Orientation);
    Normalize(&Q);
    v3 Eulers = QuaternionToEuler(Q);
    m4 Orientation = RotateAroundY(Eulers.y)*RotateAroundZ(Eulers.z)*RotateAroundX(Eulers.x); 
    
    RB->Orientation = Orientation;
#endif
    
    RB->InverseOrientation = RB->Orientation;
    Transpose(&RB->InverseOrientation);
    
}

inline void
UpdateInertiaTensor(rigid_body *RB)
{
    RB->GlobalInverseInertiaTensor = RB->Orientation*RB->Collider.LocalInertiaTensor*RB->InverseOrientation;
}

internal void
CalculatePlaneCollision(collision_box_list CollisionBoxList, v3 *LinearV, v3 *GlobalP, v3 *AngularV, m4 *Orientation)
{
    if(LengthSquared(*LinearV) != 0.0f)
    {
        For(1)//CollisionBoxList->Count)
        {
            collision_box *CBox = CollisionBoxList.Boxes[It];
            
            
            if(GlobalP->x < CBox->AABB.Max.x && 
               GlobalP->x > CBox->AABB.Min.x && 
               GlobalP->z < CBox->AABB.Max.z && 
               GlobalP->z > CBox->AABB.Min.z )
            {
                plane CollisionPlane = {CBox->AABB.Max, {0, 1, 0}};
                
                v3 IntersectionP = RayPlaneIntersection({*GlobalP, *LinearV}, CollisionPlane);
                r32 PlaneDist = Dot(*GlobalP-IntersectionP, CollisionPlane.Normal);
                if(PlaneDist<0)
                {
                    *GlobalP = Reflect(*GlobalP-IntersectionP, CollisionPlane.Normal)+IntersectionP;
                    *LinearV = Reflect(*LinearV, CollisionPlane.Normal)*CBox->Bounciness;
                    *AngularV = -Reflect(*AngularV, CollisionPlane.Normal)*CBox->Bounciness;
                    Transpose(Orientation);
                }
            }
        }
    }
}

internal void
StepPhysics(physics_infos *Physics, rigid_body *RB, r32 dTime, collision_box_list ColBoxesList)
{
    // Linear velocity
    v3 NewLinearVelocity = RB->LinearVelocity + RB->InverseMass*(RB->ForceAccumulator*dTime);
    NewLinearVelocity *= 1-Physics->AirFriction;
    // Integrate position
    v3 NewGlobalCentroid = RB->GlobalCentroid + NewLinearVelocity*dTime;
    Zero(&RB->ForceAccumulator);
    
    // Angular velocity
    v3 NewAngularVelocity = RB->AngularVelocity + RB->GlobalInverseInertiaTensor*(RB->TorqueAccumulator*dTime);
    NewAngularVelocity *= 1-(Physics->AirFriction*0.75f);
    Zero(&RB->TorqueAccumulator);
    m4 NewRotation = Identity();
    
    r32 Angle = Length(RB->AngularVelocity)*dTime;
    if(Angle != 0.0f)
    {
        // Integrate orientation
        v3 Axis   = Normalize(NewAngularVelocity);
        RotateAroundAxis(NewRotation, Axis, Angle);
    }
    
    if(RB->LinearVelocity != NewLinearVelocity ||
       RB->GlobalCentroid != NewGlobalCentroid ||
       RB->AngularVelocity != NewAngularVelocity ||
       !IsIdentity(&NewRotation))
    {
        collision_box NewCollider = RB->Collider;
        m4 NewOrientation = NewRotation*RB->Orientation;
        
        UpdateCollider(&NewCollider, &NewGlobalCentroid, &NewOrientation);
        collision_box_list PotentialCols = CollectPotentialCollisions(ColBoxesList, &NewCollider);
        
        For(PotentialCols.Count)
        {
            v3 Penetration = {};
            if(GJK(&RB->Collider, PotentialCols.Boxes[It], &Penetration))
            {
                //printf("Hit: %d, with ", PotentialCols.Boxes[It]->ID);
                //MathPrintln(Penetration);
            }
        }
        
        CalculatePlaneCollision(ColBoxesList, &NewLinearVelocity, &NewGlobalCentroid, &NewAngularVelocity, &NewRotation);
        
        RB->LinearVelocity  = NewLinearVelocity;
        RB->GlobalCentroid  = NewGlobalCentroid;
        RB->AngularVelocity = NewAngularVelocity;
        RB->Orientation     = NewRotation*RB->Orientation;
        UpdateOrientation(RB);
        UpdateCollider(&RB->Collider, &NewGlobalCentroid, &NewOrientation);
        
        UpdatePositionFromGlobalCentroid(RB);
        UpdateInertiaTensor(RB);
    }
}








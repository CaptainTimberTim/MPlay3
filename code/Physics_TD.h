#pragma once
#include "Math_TD.h"
#include "GameBasics_TD.h"

// NOTE(TD):: Basic equations of motion:      P' = new position
//                                            P  = old position
//                                            V' = new velocity
//            P' = 0.5*dT^2 + V'*dT + P       V  = old velocity
//            V' = A*dT + V                   A  = acceleration
//            A  = F/M + F                    F  = forces, like gravity
//            F  = -V*FP                      M  = mass
//                                            dT = delta time
//                                            FP = ~friction percentage

struct physics_infos
{
    b32 DoPhysics;
    
    v3 Gravity;
    r32 AirFriction;
    
    u32 CollisionBoxCount;
};

struct aa_bounding_box
{
    v3 Min;
    v3 Max;
};

struct collision_box
{
    u32 ID;
    b32 IsStatic;
    
    v3 Center;
    v3 Extends;
    m4 Orientation;
    v3 Corners[8];
    
    m4 LocalInertiaTensor;
    aa_bounding_box AABB;
    
    r32 Bounciness;
};

struct collision_box_list
{
    collision_box *Boxes[10];
    u32 Count;
};

struct rigid_body
{
    r32 Mass;
    r32 InverseMass;
    //m4 LocalInverseInertiaTensor;
    m4 GlobalInverseInertiaTensor;
    
    v3 LocalCentroid;
    v3 GlobalCentroid;
    
    v3 Position;
    m4 Orientation;
    m4 InverseOrientation;
    v3 LinearVelocity;
    v3 AngularVelocity;
    
    v3 ForceAccumulator;
    v3 TorqueAccumulator;
    
    collision_box Collider;
};

struct simplex
{
    v3 A;
    v3 B;
    v3 C;
    v3 D;
    u32 Count;
};

union face
{
    struct
    {
        v3 Points[3];
        v3 N;
    };
    struct
    {
        v3 A, B, C;
        v3 N;
    };
    v3 E[4];
};

struct edge
{
    v3 A;
    v3 B;
};

struct collision_info
{
    v3 PenetrationDepth;
    
};

#define GJK_MAX_ITERATIONS 64
#define EPA_MAX_ITERATIONS 64*2
#define EPA_MAX_FACES 64*2
#define EPA_MAX_EDGES 32*2
#define EPA_TOLERANCE 0.001f

inline rigid_body CreateRigidBody(r32 Mass, m4 LocalInertiaTensor, v3 LocalCentroid, v3 Position, m4 Orientation);
inline void       UpdateGlobalCentroidFromPosition(rigid_body *RB);
inline void       UpdatePositionFromGlobalCentroid(rigid_body *RB);
inline v3         LocalToGlobal(rigid_body *RB, v3 P);
inline v3         GlobalToLocal(rigid_body *RB, v3 P);
inline v3         LocalToGlobalVector(rigid_body *RB, v3 V);
inline v3         GlobalToLocalVector(rigid_body *RB, v3 V);
inline void       ApplyForce(rigid_body *RB, v3 Force, v3 ForceAt);
inline void       UpdateOrientation(rigid_body *RB);
inline void       UpdateInertiaTensor(rigid_body *RB);
internal void     CalculatePlaneCollision(collision_box_list CollisionBoxList, v3 *LinearV, v3 *GlobalP, v3 *AngularV, m4 *Orientation);
internal void     StepPhysics(physics_infos *Physics, rigid_body *RB, r32 dTime, collision_box_list ColBoxesList);




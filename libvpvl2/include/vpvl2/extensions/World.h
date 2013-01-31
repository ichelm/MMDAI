/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2013  hkrn                                    */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the MMDAI project team nor the names of     */
/*   its contributors may be used to endorse or promote products     */
/*   derived from this software without specific prior written       */
/*   permission.                                                     */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

#pragma once
#ifndef VPVL2_EXTENSIONS_WORLD_H_
#define VPVL2_EXTENSIONS_WORLD_H_

#include <vpvl2/Common.h>

/* Bullet Physics */
#include <BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h>

#ifdef VPVL2_EXPORT_EXTENSION_API
#define VPVL2_EXTENSION_API VPVL2_API
#elif !defined(VPVL2_EXTENSION_API)
#define VPVL2_EXTENSION_API
#endif

class btCollisionDispatcher;
class btDiscreteDynamicsWorld;
class btDbvtBroadphase;
class btRigidBody;
class btSequentialImpulseConstraintSolver;

namespace vpvl2
{
class IModel;

namespace extensions
{

class VPVL2_API World {
public:
    World();
    ~World();
    const vpvl2::Vector3 gravity() const;
    btDiscreteDynamicsWorld *dynamicWorldRef() const;
    void setGravity(const vpvl2::Vector3 &value);
    unsigned long randSeed() const;
    void setRandSeed(unsigned long value);
    void setPreferredFPS(const vpvl2::Scalar &value) ;
    void addModel(IModel *value);
    void removeModel(IModel *value);
    void addRigidBody(btRigidBody *value);
    void removeRigidBody(btRigidBody *value);
    void stepSimulation(const vpvl2::Scalar &delta);

private:
    btDefaultCollisionConfiguration m_config;
    btCollisionDispatcher *m_dispatcher;
    btDbvtBroadphase *m_broadphase;
    btSequentialImpulseConstraintSolver *m_solver;
    btDiscreteDynamicsWorld *m_world;
    vpvl2::Scalar m_motionFPS;
    Array<IModel *> m_modelRefs;
    Scalar m_fixedTimeStep;
    int m_maxSubSteps;

    VPVL2_DISABLE_COPY_AND_ASSIGN(World)
};

} /* namespace extensions */
} /* namespace vpvl2 */

#endif

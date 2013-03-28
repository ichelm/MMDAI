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

#include "vpvl2/vpvl2.h"
#include "vpvl2/internal/util.h"

#include "vpvl2/pmx/Joint.h"

namespace
{

#pragma pack(push, 1)

struct JointUnit
{
    float position[3];
    float rotation[3];
    float positionLowerLimit[3];
    float positionUpperLimit[3];
    float rotationLowerLimit[3];
    float rotationUpperLimit[3];
    float positionStiffness[3];
    float rotationStiffness[3];
};

#pragma pack(pop)

}

namespace vpvl2
{
namespace pmx
{

Joint::Joint()
    : internal::BaseJoint()
{
}

Joint::~Joint()
{
}

bool Joint::preparse(uint8_t *&ptr, size_t &rest, Model::DataInfo &info)
{
    size_t njoints, rigidBodyIndexSize = info.rigidBodyIndexSize * 2;
    if (!internal::size32(ptr, rest, njoints)) {
        VPVL2_LOG(LOG(ERROR) << "Invalid size of PMX joints detected: size=" << njoints << " rest=" << rest);
        return false;
    }
    info.jointsPtr = ptr;
    for (size_t i = 0; i < njoints; i++) {
        size_t size;
        uint8_t *namePtr;
        /* name in Japanese */
        if (!internal::sizeText(ptr, rest, namePtr, size)) {
            VPVL2_LOG(LOG(ERROR) << "Invalid size of PMX joint name in Japanese detected: index=" << i << " size=" << size << " rest=" << rest);
            return false;
        }
        /* name in English */
        if (!internal::sizeText(ptr, rest, namePtr, size)) {
            VPVL2_LOG(LOG(ERROR) << "Invalid size of PMX joint name in English detected: index=" << i << " size=" << size << " rest=" << rest);
            return false;
        }
        size_t type;
        if (!internal::size8(ptr, rest, type)) {
            VPVL2_LOG(LOG(ERROR) << "Invalid size of PMX joint type detected: index=" << i << " ptr=" << static_cast<const void *>(ptr) << " rest=" << rest);
            return false;
        }
        switch (type) {
        case 0: {
            if (!internal::validateSize(ptr, rigidBodyIndexSize + sizeof(JointUnit), rest)) {
                VPVL2_LOG(LOG(ERROR) << "Invalid size of PMX joint unit detected: index=" << i << " ptr=" << static_cast<const void *>(ptr) << " rest=" << rest);
                return false;
            }
            break;
        }
        default:
            VPVL2_LOG(LOG(ERROR) << "Invalid PMX Joint type specified: index=" << i << " type=" << type);
            return false;
        }
    }
    info.jointsCount = njoints;
    return true;
}

bool Joint::loadJoints(const Array<Joint *> &joints, const Array<RigidBody *> &rigidBodies)
{
    const int njoints = joints.count();
    const int nRigidBodies = rigidBodies.count();
    for (int i = 0; i < njoints; i++) {
        Joint *joint = joints[i];
        const int rigidBodyIndex1 = joint->m_rigidBodyIndex1;
        if (rigidBodyIndex1 >= 0) {
            if (rigidBodyIndex1 >= nRigidBodies) {
                VPVL2_LOG(LOG(ERROR) << "Invalid rigidBodyIndex1 specified: index=" << i << " body=" << rigidBodyIndex1);
                return false;
            }
            else {
                joint->m_rigidBody1Ref = rigidBodies[rigidBodyIndex1];
            }
        }
        const int rigidBodyIndex2 = joint->m_rigidBodyIndex2;
        if (rigidBodyIndex2 >= 0) {
            if (rigidBodyIndex2 >= nRigidBodies) {
                VPVL2_LOG(LOG(ERROR) << "Invalid rigidBodyIndex2 specified: index=" << i << " body=" << rigidBodyIndex2);
                return false;
            }
            else {
                joint->m_rigidBody2Ref = rigidBodies[rigidBodyIndex2];
            }
        }
        joint->build(i);
    }
    return true;
}

size_t Joint::estimateTotalSize(const Array<Joint *> &joints, const Model::DataInfo &info)
{
    const int njoints = joints.count();
    size_t size = 0;
    size += sizeof(njoints);
    for (int i = 0; i < njoints; i++) {
        Joint *joint = joints[i];
        size += joint->estimateSize(info);
    }
    return size;
}

void Joint::read(const uint8_t *data, const Model::DataInfo &info, size_t &size)
{
    uint8_t *namePtr, *ptr = const_cast<uint8_t *>(data), *start = ptr;
    size_t nNameSize, rest = SIZE_MAX;
    IEncoding *encoding = info.encoding;
    internal::sizeText(ptr, rest, namePtr, nNameSize);
    internal::setStringDirect(encoding->toString(namePtr, nNameSize, info.codec), m_name);
    VPVL2_LOG(VLOG(3) << "PMXJoint: name=" << reinterpret_cast<const char *>(m_name->toByteArray()));
    internal::sizeText(ptr, rest, namePtr, nNameSize);
    internal::setStringDirect(encoding->toString(namePtr, nNameSize, info.codec), m_englishName);
    VPVL2_LOG(VLOG(3) << "PMXJoint: englishName=" << reinterpret_cast<const char *>(m_englishName->toByteArray()));
    internal::size8(ptr, rest, nNameSize);
    m_type = static_cast<Type>(nNameSize);
    m_rigidBodyIndex1 = internal::readSignedIndex(ptr, info.rigidBodyIndexSize);
    m_rigidBodyIndex2 = internal::readSignedIndex(ptr, info.rigidBodyIndexSize);
    VPVL2_LOG(VLOG(3) << "PMXJoint: type=" << m_type << " rigidBodyIndex1=" << m_rigidBodyIndex1 << " rigidBodyIndex2=" << m_rigidBodyIndex2);
    JointUnit unit;
    internal::getData(ptr, unit);
    internal::setPositionRaw(unit.position, m_position);
    VPVL2_LOG(VLOG(3) << "PMXJoint: position=" << m_position.x() << "," << m_position.y() << "," << m_position.z());
    internal::setPositionRaw(unit.rotation, m_rotation);
    VPVL2_LOG(VLOG(3) << "PMXJoint: rotation=" << m_rotation.x() << "," << m_rotation.y() << "," << m_rotation.z());
    internal::setPositionRaw(unit.positionLowerLimit, m_positionLowerLimit);
    VPVL2_LOG(VLOG(3) << "PMXJoint: positionLowerLimit=" << m_positionLowerLimit.x() << "," << m_positionLowerLimit.y() << "," << m_positionLowerLimit.z());
    internal::setPositionRaw(unit.rotationLowerLimit, m_rotationLowerLimit);
    VPVL2_LOG(VLOG(3) << "PMXJoint: rotationLowerLimit=" << m_rotationLowerLimit.x() << "," << m_rotationLowerLimit.y() << "," << m_rotationLowerLimit.z());
    internal::setPositionRaw(unit.positionUpperLimit, m_positionUpperLimit);
    VPVL2_LOG(VLOG(3) << "PMXJoint: positionUpperLimit=" << m_positionUpperLimit.x() << "," << m_positionUpperLimit.y() << "," << m_positionUpperLimit.z());
    internal::setPositionRaw(unit.rotationUpperLimit, m_rotationUpperLimit);
    VPVL2_LOG(VLOG(3) << "PMXJoint: rotationUpperLimit=" << m_rotationUpperLimit.x() << "," << m_rotationUpperLimit.y() << "," << m_rotationUpperLimit.z());
    internal::setPositionRaw(unit.positionStiffness, m_positionStiffness);
    VPVL2_LOG(VLOG(3) << "PMXJoint: positionStiffness=" << m_positionStiffness.x() << "," << m_positionStiffness.y() << "," << m_positionStiffness.z());
    internal::setPositionRaw(unit.rotationStiffness, m_rotationStiffness);
    VPVL2_LOG(VLOG(3) << "PMXJoint: rotationStiffness=" << m_rotationStiffness.x() << "," << m_rotationStiffness.y() << "," << m_rotationStiffness.z());
    ptr += sizeof(unit);
    size = ptr - start;
}

void Joint::write(uint8_t *data, const Model::DataInfo &info) const
{
    internal::writeString(m_name, info.codec, data);
    internal::writeString(m_englishName, info.codec, data);
    uint8_t type = m_type;
    internal::writeBytes(reinterpret_cast<const uint8_t *>(&type), sizeof(type), data);
    size_t rigidBodyIndexSize = info.rigidBodyIndexSize;
    internal::writeSignedIndex(m_rigidBodyIndex1, rigidBodyIndexSize, data);
    internal::writeSignedIndex(m_rigidBodyIndex2, rigidBodyIndexSize, data);
    JointUnit ju;
    internal::getPositionRaw(m_position, ju.position);
    internal::getPositionRaw(m_rotation, ju.rotation);
    internal::getPositionRaw(m_positionLowerLimit, ju.positionLowerLimit);
    internal::getPositionRaw(m_rotationLowerLimit, ju.rotationLowerLimit);
    internal::getPositionRaw(m_positionUpperLimit, ju.positionUpperLimit);
    internal::getPositionRaw(m_rotationUpperLimit, ju.rotationUpperLimit);
    internal::getPositionRaw(m_positionStiffness, ju.positionStiffness);
    internal::getPositionRaw(m_rotationStiffness, ju.rotationStiffness);
    internal::writeBytes(reinterpret_cast<const uint8_t *>(&ju), sizeof(ju), data);
}

size_t Joint::estimateSize(const Model::DataInfo &info) const
{
    size_t size = 0;
    size += internal::estimateSize(m_name, info.codec);
    size += internal::estimateSize(m_englishName, info.codec);
    size += sizeof(uint8_t);
    size += info.rigidBodyIndexSize * 2;
    size += sizeof(JointUnit);
    return size;
}

} /* namespace pmx */
} /* namespace vpvl2 */


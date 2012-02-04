/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2012  hkrn                                    */
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

namespace vpvl2
{
namespace pmx
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

Joint::Joint()
    : m_name(0),
      m_englishName(0),
      m_position(kZeroV),
      m_rotation(kZeroV),
      m_positionLowerLimit(kZeroV),
      m_rotationLowerLimit(kZeroV),
      m_positionUpperLimit(kZeroV),
      m_rotationUpperLimit(kZeroV),
      m_positionStiffness(kZeroV),
      m_rotationStiffness(kZeroV),
      m_rigidBodyIndex1(0),
      m_rigidBodyIndex2(0)
{
}

Joint::~Joint()
{
    delete m_name;
    m_name = 0;
    delete m_englishName;
    m_englishName = 0;
    m_position.setZero();
    m_rotation.setZero();
    m_positionLowerLimit.setZero();
    m_rotationLowerLimit.setZero();
    m_positionUpperLimit.setZero();
    m_rotationUpperLimit.setZero();
    m_positionStiffness.setZero();
    m_rotationStiffness.setZero();
    m_rigidBodyIndex1 = 0;
    m_rigidBodyIndex2 = 0;
}

bool Joint::preparse(uint8_t *&ptr, size_t &rest, Model::DataInfo &info)
{
    size_t size;
    if (!internal::size32(ptr, rest, size)) {
        return false;
    }
    info.jointsPtr = ptr;
    for (size_t i = 0; i < size; i++) {
        size_t nNameSize;
        uint8_t *namePtr;
        /* name in Japanese */
        if (!internal::sizeText(ptr, rest, namePtr, nNameSize)) {
            return false;
        }
        /* name in English */
        if (!internal::sizeText(ptr, rest, namePtr, nNameSize)) {
            return false;
        }
        size_t type;
        if (!internal::size8(ptr, rest, type)) {
            return false;
        }
        switch (type) {
        case 0:
            if (!internal::validateSize(ptr, info.rigidBodyIndexSize * 2 + sizeof(JointUnit), rest)) {
                return false;
            }
            break;
        default:
            assert(0);
            return false;
        }
    }
    info.jointsCount = size;
    return true;
}

void Joint::read(const uint8_t *data, const Model::DataInfo &info, size_t &size)
{
    uint8_t *namePtr, *ptr = const_cast<uint8_t *>(data), *start = ptr;
    size_t nNameSize, rest = SIZE_MAX;
    internal::sizeText(ptr, rest, namePtr, nNameSize);
    m_name = new StaticString(namePtr, nNameSize);
    internal::sizeText(ptr, rest, namePtr, nNameSize);
    m_englishName = new StaticString(namePtr, nNameSize);
    internal::size8(ptr, rest, nNameSize);
    switch (nNameSize) {
    case 0: {
        m_rigidBodyIndex1 = internal::variantIndex(ptr, info.rigidBodyIndexSize);
        m_rigidBodyIndex2 = internal::variantIndex(ptr, info.rigidBodyIndexSize);
        const JointUnit &unit = *reinterpret_cast<JointUnit *>(ptr);
        m_position.setValue(unit.position[0], unit.position[1], unit.position[2]);
        m_rotation.setValue(unit.rotation[0], unit.rotation[1], unit.rotation[2]);
        m_positionLowerLimit.setValue(unit.positionLowerLimit[0], unit.positionLowerLimit[1], unit.positionLowerLimit[2]);
        m_rotationLowerLimit.setValue(unit.rotationLowerLimit[0], unit.rotationLowerLimit[1], unit.rotationLowerLimit[2]);
        m_positionUpperLimit.setValue(unit.positionUpperLimit[0], unit.positionUpperLimit[1], unit.positionUpperLimit[2]);
        m_rotationUpperLimit.setValue(unit.rotationUpperLimit[0], unit.rotationUpperLimit[1], unit.rotationUpperLimit[2]);
        m_positionStiffness.setValue(unit.positionStiffness[0], unit.positionStiffness[1], unit.positionStiffness[2]);
        m_rotationStiffness.setValue(unit.rotationStiffness[0], unit.rotationStiffness[1], unit.rotationStiffness[2]);
        ptr += sizeof(unit);
        break;
    }
    default: {
        assert(0);
        return;
    }
    }
    size = ptr - start;
}

void Joint::write(uint8_t *data) const
{
}

} /* namespace pmx */
} /* namespace vpvl2 */

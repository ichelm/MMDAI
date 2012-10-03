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

#include "vpvl2/pmx/Bone.h"
#include "vpvl2/pmx/Joint.h"
#include "vpvl2/pmx/Label.h"
#include "vpvl2/pmx/Material.h"
#include "vpvl2/pmx/Model.h"
#include "vpvl2/pmx/Morph.h"
#include "vpvl2/pmx/RigidBody.h"
#include "vpvl2/pmx/Vertex.h"

#ifndef VPVL2_NO_BULLET
#include <btBulletDynamicsCommon.h>
#else
BT_DECLARE_HANDLE(btDiscreteDynamicsWorld);
#endif

namespace {

using namespace vpvl2;

#pragma pack(push, 1)

struct Header
{
    uint8_t signature[4];
    float version;
};

#pragma pack(pop)

struct StaticVertexBuffer : public IModel::IStaticVertexBuffer {
    struct Buffer {
        Buffer() {}
        Buffer(const IVertex *vertex) {
            IBone *bone1 = vertex->bone(0),
                    *bone2 = vertex->bone(1),
                    *bone3 = vertex->bone(2),
                    *bone4 = vertex->bone(3);
            boneIndices.setValue(bone1 ? bone1->index() : -1,
                                 bone2 ? bone2->index() : -1,
                                 bone3 ? bone3->index() : -1,
                                 bone4 ? bone4->index() : -1);
            boneWeights.setValue(vertex->weight(0),
                                 vertex->weight(1),
                                 vertex->weight(2),
                                 vertex->weight(3));
            texcoord = vertex->textureCoord();
        }
        Vector3 texcoord;
        Vector4 boneIndices;
        Vector4 boneWeights;
    };

    StaticVertexBuffer(const pmx::Model *model)
        : modelRef(model)
    {
        Array<IVertex *> vertices;
        model->getVertexRefs(vertices);
        const int nvertices = vertices.count();
        for (int i = 0; i < nvertices; i++) {
            IVertex *vertex = vertices[i];
            buffer.add(Buffer(vertex));
        }
    }

    const void *bytes() const {
        return &buffer[0];
    }
    size_t size() const {
        return strideSize() * buffer.count();
    }
    size_t strideOffset(StrideType type) const {
        static const Buffer v;
        const uint8_t *base = reinterpret_cast<const uint8_t *>(&v.texcoord);
        switch (type) {
        case kBoneIndexStride:
            return reinterpret_cast<const uint8_t *>(&v.boneIndices) - base;
        case kBoneWeightStride:
            return reinterpret_cast<const uint8_t *>(&v.boneWeights) - base;
        case kTextureCoordStride:
            return reinterpret_cast<const uint8_t *>(&v.texcoord) - base;
        case kVertexStride:
        case kNormalStride:
        case kMorphDeltaStride:
        case kEdgeSizeStride:
        case kEdgeVertexStride:
        case kUVA0Stride:
        case kUVA1Stride:
        case kUVA2Stride:
        case kUVA3Stride:
        case kUVA4Stride:
        case kVertexIndexStride:
        case kIndexStride:
        default:
            return 0;
        }
    }
    size_t strideSize() const {
        return sizeof(Buffer);
    }

    const IModel *modelRef;
    Array<Buffer> buffer;
};

struct DynamicVertexBuffer : public IModel::IDynamicVertexBuffer {
    struct Buffer {
        Buffer() {}
        Buffer(const IVertex *vertex, int index) {
            position = vertex->origin();
            normal = vertex->normal();
            normal[3] = vertex->edgeSize();
            delta = vertex->delta();
            edge.setZero();
            edge[3] = Scalar(index);
            uva0 = vertex->uv(0);
            uva1 = vertex->uv(1);
            uva2 = vertex->uv(2);
            uva3 = vertex->uv(3);
            uva4 = vertex->uv(4);
        }
        Vector3 position;
        Vector3 normal;
        Vector3 delta;
        Vector3 edge;
        Vector4 uva0;
        Vector4 uva1;
        Vector4 uva2;
        Vector4 uva3;
        Vector4 uva4;
    };

    DynamicVertexBuffer(const pmx::Model *model, const IModel::IIndexBuffer *indexBuffer)
        : modelRef(model),
          indexBufferRef(indexBuffer),
          enableSkinning(true)
    {
        model->getMaterialRefs(materials);
        model->getVertexRefs(vertices);
        const int nvertices = vertices.count();
        for (int i = 0; i < nvertices; i++) {
            IVertex *vertex = vertices[i];
            buffer.add(Buffer(vertex, i));
        }
    }

    void update(const Vector3 &cameraPosition, Vector3 &aabbMin, Vector3 &aabbMax) {
        update(cameraPosition, &buffer[0], aabbMin, aabbMax);
    }
    const void *bytes() const {
        return &buffer[0];
    }
    size_t size() const {
        return strideSize() * buffer.count();
    }
    size_t strideOffset(StrideType type) const {
        static const Buffer v;
        const uint8_t *base = reinterpret_cast<const uint8_t *>(&v.position);
        switch (type) {
        case kVertexStride:
            return reinterpret_cast<const uint8_t *>(&v.position) - base;
        case kNormalStride:
            return reinterpret_cast<const uint8_t *>(&v.normal) - base;
        case kMorphDeltaStride:
            return reinterpret_cast<const uint8_t *>(&v.delta) - base;
        case kEdgeSizeStride:
            return reinterpret_cast<const uint8_t *>(&v.normal[3]) - base;
        case kEdgeVertexStride:
            return reinterpret_cast<const uint8_t *>(&v.edge[3]) - base;
        case kUVA0Stride:
            return reinterpret_cast<const uint8_t *>(&v.uva0) - base;
        case kUVA1Stride:
            return reinterpret_cast<const uint8_t *>(&v.uva1) - base;
        case kUVA2Stride:
            return reinterpret_cast<const uint8_t *>(&v.uva2) - base;
        case kUVA3Stride:
            return reinterpret_cast<const uint8_t *>(&v.uva3) - base;
        case kUVA4Stride:
            return reinterpret_cast<const uint8_t *>(&v.uva4) - base;
        case kVertexIndexStride:
        case kBoneIndexStride:
        case kBoneWeightStride:
        case kTextureCoordStride:
        case kIndexStride:
        default:
            return 0;
        }
    }
    size_t strideSize() const {
        return sizeof(Buffer);
    }
    void update(const Vector3 &cameraPosition, void *address, Vector3 &aabbMin, Vector3 &aabbMax) {
        if (enableSkinning) {
            const Scalar &esf = modelRef->edgeScaleFactor(cameraPosition);
            const int nmaterials = materials.count();
            Buffer *bufferPtr = static_cast<Buffer *>(address);
            Vector3 position, normal;
            int offset = 0;
            for (int i = 0; i < nmaterials; i++) {
                const IMaterial *material = materials[i];
                const int nindices = material->indices(), offsetTo = offset + nindices;
                const float materialEdgeSize = material->edgeSize();
                for (int j = offset; j < offsetTo; j++) {
                    const int index = indexBufferRef->indexAt(j);
                    IVertex *vertex = vertices[index];
                    Buffer &v = bufferPtr[index];
                    const float edgeSize = vertex->edgeSize();
                    vertex->performSkinning(position, normal);
                    v.position = position;
                    v.normal = normal;
                    v.delta = vertex->delta();
                    v.edge = position + normal * edgeSize * materialEdgeSize * esf;
                    v.edge[3] = Scalar(i);
                    v.uva0 = vertex->uv(0);
                    v.uva1 = vertex->uv(1);
                    v.uva2 = vertex->uv(2);
                    v.uva3 = vertex->uv(3);
                    v.uva4 = vertex->uv(4);
                    aabbMin.setMin(position);
                    aabbMax.setMax(position);
                }
                offset += nindices;
            }
        }
        else {
            const int nvertices = vertices.count();
            for (int i = 0; i < nvertices; i++) {
                IVertex *vertex = vertices[i];
                Buffer &v = buffer[i];
                v.delta = vertex->delta();
            }
            aabbMin.setZero();
            aabbMax.setZero();
        }
    }
    void setSkinningEnable(bool value) {
        enableSkinning = value;
    }

    const IModel *modelRef;
    const IModel::IIndexBuffer *indexBufferRef;
    Array<Buffer> buffer;
    Array<IMaterial *> materials;
    Array<IVertex *> vertices;
    bool enableSkinning;
};

struct IndexBuffer : public IModel::IIndexBuffer {
    IndexBuffer(const Array<int> &indices, const int nvertices)
        : indexType(kIndex32),
          indices32Ptr(0),
          nindices(indices.count())
    {
        if (nindices < 65536) {
            indexType = kIndex16;
            indices16Ptr = new uint16_t[nindices];
        }
        else if (nindices < 256) {
            indexType = kIndex8;
            indices8Ptr = new uint8_t[nindices];
        }
        else {
            indices32Ptr = new int[nindices];
        }
        for (int i = 0; i < nindices; i++) {
            int index = indices[i];
            if (index >= 0 && index < nvertices) {
                setIndexAt(i, index);
            }
            else {
                setIndexAt(i, 0);
            }
        }
#ifdef VPVL2_COORDINATE_OPENGL
        for (int i = 0; i < nindices; i += 3) {
            switch (indexType) {
            case kIndex32:
                btSwap(indices32Ptr[i], indices32Ptr[i + 1]);
                break;
            case kIndex16:
                btSwap(indices16Ptr[i], indices16Ptr[i + 1]);
                break;
            case kIndex8:
                btSwap(indices8Ptr[i], indices8Ptr[i + 1]);
                break;
            case kMaxIndexType:
            default:
                break;
            }
        }
#endif
    }
    ~IndexBuffer() {
        switch (indexType) {
        case kIndex32:
            delete[] indices32Ptr;
            indices32Ptr = 0;
            break;
        case kIndex16:
            delete[] indices16Ptr;
            indices16Ptr = 0;
            break;
        case kIndex8:
            delete[] indices8Ptr;
            indices8Ptr = 0;
            break;
        case kMaxIndexType:
        default:
            break;
        }
    }

    const void *bytes() const {
        switch (indexType) {
        case kIndex32:
            return indices32Ptr;
        case kIndex16:
            return indices16Ptr;
        case kIndex8:
            return indices8Ptr;
        case kMaxIndexType:
        default:
            return 0;
        }
    }
    size_t size() const {
        return strideSize() * nindices;
    }
    size_t strideOffset(StrideType /* type */) const {
        return 0;
    }
    size_t strideSize() const {
        switch (indexType) {
        case kIndex32:
            return sizeof(int);
        case kIndex16:
            return sizeof(uint16_t);
        case kIndex8:
            return sizeof(uint8_t);
        case kMaxIndexType:
        default:
            return 0;
        }
    }
    int indexAt(int index) const {
        switch (indexType) {
        case kIndex32:
            return indices32Ptr[index];
        case kIndex16:
            return indices16Ptr[index];
        case kIndex8:
            return indices8Ptr[index];
        case kMaxIndexType:
        default:
            return 0;
        }
    }
    Type type() const {
        return indexType;
    }

    void setIndexAt(int i, int value) {
        switch (indexType) {
        case kIndex32:
            indices32Ptr[i] = value;
            break;
        case kIndex16:
            indices16Ptr[i] = uint16_t(value);
            break;
        case kIndex8:
            indices8Ptr[i] = uint8_t(value);
            break;
        case kMaxIndexType:
        default:
            break;
        }
    }
    IModel::IIndexBuffer::Type indexType;
    union {
        int      *indices32Ptr;
        uint16_t *indices16Ptr;
        uint8_t  *indices8Ptr;
    };
    int nindices;
};

struct MatrixBuffer : public IModel::IMatrixBuffer {
    typedef btAlignedObjectArray<int> BoneIndices;
    typedef btAlignedObjectArray<BoneIndices> MeshBoneIndices;
    typedef btAlignedObjectArray<Transform> MeshLocalTransforms;
    typedef Array<float *> MeshMatrices;
    struct SkinningMeshes {
        MeshBoneIndices bones;
        MeshLocalTransforms transforms;
        MeshMatrices matrices;
        BoneIndices bdef1;
        BoneIndices bdef2;
        BoneIndices bdef4;
        BoneIndices sdef;
        ~SkinningMeshes() { matrices.releaseArrayAll(); }
    };

    MatrixBuffer(const pmx::Model *model, const IndexBuffer *indexBuffer, DynamicVertexBuffer *dynamicBuffer)
        : modelRef(model),
          indexBufferRef(indexBuffer),
          dynamicBufferRef(dynamicBuffer)
    {
        model->getBoneRefs(bones);
        model->getMaterialRefs(materials);
        model->getVertexRefs(vertices);
        initialize();
    }

    void update() {
        const int nbones = bones.count();
        MeshLocalTransforms &transforms = meshes.transforms;
        for (int i = 0; i < nbones; i++) {
            const IBone *bone = bones[i];
            transforms[i] = bone->localTransform();
        }
        const int nmaterials = materials.count();
        for (int i = 0; i < nmaterials; i++) {
            const BoneIndices &boneIndices = meshes.bones[i];
            const int nBoneIndices = boneIndices.size();
            Scalar *matrices = meshes.matrices[i];
            for (int j = 0; j < nBoneIndices; j++) {
                const int boneIndex = boneIndices[j];
                const Transform &transform = transforms[boneIndex];
                transform.getOpenGLMatrix(&matrices[j * 16]);
            }
        }
        const int nvertices = vertices.count();
        for (int i = 0; i < nvertices; i++)
            dynamicBufferRef->buffer[i].position = vertices[i]->origin(); // + m_vertices[i]->delta();
    }
    const float *bytes(int materialIndex) const {
        int nmatrices = meshes.matrices.count();
        return internal::checkBound(materialIndex, 0, nmatrices) ? meshes.matrices[materialIndex] : 0;
    }
    size_t size(int materialIndex) const {
        int nbones = meshes.bones.size();
        return internal::checkBound(materialIndex, 0, nbones) ? meshes.bones[materialIndex].size() : 0;
    }

    void initialize() {
        const int nmaterials = materials.count();
        btHashMap<btHashInt, int> set;
        BoneIndices boneIndices;
        meshes.transforms.resize(bones.count());
        int offset = 0;
        for (int i = 0; i < nmaterials; i++) {
            const IMaterial *material = materials[i];
            const int nindices = material->indices();
            for (int j = 0; j < nindices; j++) {
                int vertexIndex = indexBufferRef->indexAt(offset + j);
                IVertex *vertex = vertices[vertexIndex];
                switch (vertex->type()) {
                case IVertex::kBdef1:
                    meshes.bdef1.push_back(vertexIndex);
                    break;
                case IVertex::kBdef2:
                    meshes.bdef2.push_back(vertexIndex);
                    break;
                case IVertex::kBdef4:
                    meshes.bdef4.push_back(vertexIndex);
                    break;
                case IVertex::kSdef:
                    meshes.sdef.push_back(vertexIndex);
                    break;
                case IVertex::kMaxType:
                default:
                    break;
                }
                dynamicBufferRef->buffer[vertexIndex].position.setW(Scalar(vertex->type()));
            }
            meshes.matrices.add(new Scalar[boneIndices.size() * 16]);
            meshes.bones.push_back(boneIndices);
            boneIndices.clear();
            set.clear();
            offset += nindices;
        }
    }

    const IModel *modelRef;
    const IndexBuffer *indexBufferRef;
    DynamicVertexBuffer *dynamicBufferRef;
    Array<IBone *> bones;
    Array<IMaterial *> materials;
    Array<IVertex *> vertices;
    SkinningMeshes meshes;
};

}

namespace vpvl2
{
namespace pmx
{

Model::Model(IEncoding *encoding)
    : m_worldRef(0),
      m_encodingRef(encoding),
      m_name(0),
      m_englishName(0),
      m_comment(0),
      m_englishComment(0),
      m_position(kZeroV3),
      m_rotation(Quaternion::getIdentity()),
      m_opacity(1),
      m_scaleFactor(1),
      m_edgeWidth(0),
      m_visible(false)
{
    internal::zerofill(&m_info, sizeof(m_info));
}

Model::~Model()
{
    release();
}

bool Model::load(const uint8_t *data, size_t size)
{
    DataInfo info;
    internal::zerofill(&info, sizeof(info));
    if (preparse(data, size, info)) {
        release();
        parseNamesAndComments(info);
        parseVertices(info);
        parseIndices(info);
        parseTextures(info);
        parseMaterials(info);
        parseBones(info);
        parseMorphs(info);
        parseLabels(info);
        parseRigidBodies(info);
        parseJoints(info);
        if (!Bone::loadBones(m_bones, m_BPSOrderedBones, m_APSOrderedBones)
                || !Material::loadMaterials(m_materials, m_textures, m_indices.count())
                || !Vertex::loadVertices(m_vertices, m_bones)
                || !Morph::loadMorphs(m_morphs, m_bones, m_materials, m_vertices)
                || !Label::loadLabels(m_labels, m_bones, m_morphs)
                || !RigidBody::loadRigidBodies(m_rigidBodies, m_bones)
                || !Joint::loadJoints(m_joints, m_rigidBodies)) {
            m_info.error = info.error;
            return false;
        }
        m_info = info;
        return true;
    }
    else {
        m_info.error = info.error;
    }
    return false;
}

void Model::save(uint8_t * /* data */) const
{
}

size_t Model::estimateSize() const
{
    size_t size = 0;
    IString::Codec codec = m_info.codec;
    size += sizeof(Header);
    size += sizeof(uint8_t) + 8;
    size += internal::estimateSize(m_name, codec);
    size += internal::estimateSize(m_englishName, codec);
    size += internal::estimateSize(m_comment, codec);
    size += internal::estimateSize(m_englishComment, codec);
    const int nvertices = m_vertices.count();
    size += sizeof(nvertices);
    for (int i = 0; i < nvertices; i++) {
        Vertex *vertex = m_vertices[i];
        size += vertex->estimateSize(m_info);
    }
    const int nindices = m_indices.count();
    size += sizeof(nindices);
    size += m_info.vertexIndexSize * nindices;
    const int ntextures = m_textures.count();
    size += sizeof(ntextures);
    for (int i = 0; i < ntextures; i++) {
        IString *texture = m_textures[i];
        size += internal::estimateSize(texture, codec);
    }
    const int nmaterials = m_materials.count();
    size += sizeof(nmaterials);
    for (int i = 0; i < nmaterials; i++) {
        Material *material = m_materials[i];
        size += material->estimateSize(m_info);
    }
    const int nbones = m_bones.count();
    size += sizeof(nbones);
    for (int i = 0; i < nbones; i++) {
        Bone *bone = m_bones[i];
        size += bone->estimateSize(m_info);
    }
    const int nmorphs = m_morphs.count();
    size += sizeof(nmorphs);
    for (int i = 0; i < nmorphs; i++) {
        Morph *morph = m_morphs[i];
        size += morph->estimateSize(m_info);
    }
    const int nlabels = m_labels.count();
    size += sizeof(nlabels);
    for (int i = 0; i < nlabels; i++) {
        Label *label = m_labels[i];
        size += label->estimateSize(m_info);
    }
    const int nbodies = m_rigidBodies.count();
    size += sizeof(nbodies);
    for (int i = 0; i < nbodies; i++) {
        RigidBody *body = m_rigidBodies[i];
        size += body->estimateSize(m_info);
    }
    const int njoints = m_joints.count();
    size += sizeof(njoints);
    for (int i = 0; i < njoints; i++) {
        Joint *joint = m_joints[i];
        size += joint->estimateSize(m_info);
    }
    return size;
}

void Model::resetVertices()
{
    const int nvertices = m_vertices.count();
    for (int i = 0; i < nvertices; i++) {
        Vertex *vertex = m_vertices[i];
        vertex->reset();
    }
}

void Model::resetMotionState()
{
    const int nRigidBodies = m_rigidBodies.count();
    for (int i = 0; i < nRigidBodies; i++) {
        RigidBody *rigidBody = m_rigidBodies[i];
        rigidBody->setKinematic(false);
    }
}

void Model::performUpdate()
{
    // update local transform matrix
    const int nbones = m_bones.count();
    for (int i = 0; i < nbones; i++) {
        Bone *bone = m_bones[i];
        bone->resetIKLink();
    }
    // before physics simulation
    const int nBPSBones = m_BPSOrderedBones.count();
    for (int i = 0; i < nBPSBones; i++) {
        Bone *bone = m_BPSOrderedBones[i];
        bone->performFullTransform();
        bone->performInverseKinematics();
    }
    for (int i = 0; i < nBPSBones; i++) {
        Bone *bone = m_BPSOrderedBones[i];
        bone->performUpdateLocalTransform();
    }
    // physics simulation
    if (m_worldRef) {
        const int nRigidBodies = m_rigidBodies.count();
        for (int i = 0; i < nRigidBodies; i++) {
            RigidBody *rigidBody = m_rigidBodies[i];
            rigidBody->performTransformBone();
        }
    }
    // after physics simulation
    const int nAPSBones = m_APSOrderedBones.count();
    for (int i = 0; i < nAPSBones; i++) {
        Bone *bone = m_APSOrderedBones[i];
        bone->performFullTransform();
        bone->performInverseKinematics();
    }
    for (int i = 0; i < nAPSBones; i++) {
        Bone *bone = m_APSOrderedBones[i];
        bone->performUpdateLocalTransform();
    }
}

void Model::joinWorld(btDiscreteDynamicsWorld *world)
{
#ifndef VPVL2_NO_BULLET
    if (!world)
        return;
    const int nRigidBodies = m_rigidBodies.count();
    for (int i = 0; i < nRigidBodies; i++) {
        RigidBody *rigidBody = m_rigidBodies[i];
        world->addRigidBody(rigidBody->body(), rigidBody->groupID(), rigidBody->collisionGroupMask());
    }
    const int njoints = m_joints.count();
    for (int i = 0; i < njoints; i++) {
        Joint *joint = m_joints[i];
        world->addConstraint(joint->constraint());
    }
    m_worldRef = world;
#endif /* VPVL2_NO_BULLET */
}

void Model::leaveWorld(btDiscreteDynamicsWorld *world)
{
#ifndef VPVL2_NO_BULLET
    if (!world)
        return;
    const int nRigidBodies = m_rigidBodies.count();
    for (int i = nRigidBodies - 1; i >= 0; i--) {
        RigidBody *rigidBody = m_rigidBodies[i];
        world->removeCollisionObject(rigidBody->body());
    }
    const int njoints = m_joints.count();
    for (int i = njoints - 1; i >= 0; i--) {
        Joint *joint = m_joints[i];
        world->removeConstraint(joint->constraint());
    }
    m_worldRef = 0;
#endif /* VPVL2_NO_BULLET */
}

IBone *Model::findBone(const IString *value) const
{
    IBone **bone = const_cast<IBone **>(m_name2boneRefs.find(value->toHashString()));
    return bone ? *bone : 0;
}

IMorph *Model::findMorph(const IString *value) const
{
    IMorph **morph = const_cast<IMorph **>(m_name2morphRefs.find(value->toHashString()));
    return morph ? *morph : 0;
}

int Model::count(ObjectType value) const
{
    switch (value) {
    case kBone: {
        return m_bones.count();
    }
    case kIK: {
        const int nbones = m_bones.count();
        int nIK = 0;
        for (int i = 0; i < nbones; i++) {
            Bone *bone = static_cast<Bone *>(m_bones[i]);
            if (bone->hasInverseKinematics())
                nIK++;
        }
        return nIK;
    }
    case kIndex: {
        return m_indices.count();
    }
    case kJoint: {
        return m_joints.count();
    }
    case kMaterial: {
        return m_materials.count();
    }
    case kMorph: {
        return m_morphs.count();
    }
    case kRigidBody: {
        return m_rigidBodies.count();
    }
    case kVertex: {
        return m_vertices.count();
    }
    default:
        return 0;
    }
}

void Model::getBoneRefs(Array<IBone *> &value) const
{
    const int nbones = m_bones.count();
    for (int i = 0; i < nbones; i++) {
        Bone *bone = m_bones[i];
        value.add(bone);
    }
}

void Model::getLabelRefs(Array<ILabel *> &value) const
{
    const int nlabels = m_labels.count();
    for (int i = 0; i < nlabels; i++) {
        ILabel *label = m_labels[i];
        value.add(label);
    }
}

void Model::getMaterialRefs(Array<IMaterial *> &value) const
{
    const int nmaterials = m_materials.count();
    for (int i = 0; i < nmaterials; i++) {
        IMaterial *material = m_materials[i];
        value.add(material);
    }
}

void Model::getMorphRefs(Array<IMorph *> &value) const
{
    const int nmorphs = m_morphs.count();
    for (int i = 0; i < nmorphs; i++) {
        Morph *morph = m_morphs[i];
        value.add(morph);
    }
}

void Model::getVertexRefs(Array<IVertex *> &value) const
{
    const int nvertices = m_vertices.count();
    for (int i = 0; i < nvertices; i++) {
        IVertex *vertex = m_vertices[i];
        value.add(vertex);
    }
}

bool Model::preparse(const uint8_t *data, size_t size, DataInfo &info)
{
    size_t rest = size;
    if (!data || sizeof(Header) > rest) {
        m_info.error = kInvalidHeaderError;
        return false;
    }
    /* header */
    uint8_t *ptr = const_cast<uint8_t *>(data);
    Header header;
    internal::getData(ptr, header);
    info.basePtr = ptr;

    /* Check the signature and version is correct */
    if (memcmp(header.signature, "PMX ", 4) != 0) {
        m_info.error = kInvalidSignatureError;
        return false;
    }

    /* version */
    if (header.version != 2.0) {
        m_info.error = kInvalidVersionError;
        return false;
    }

    /* flags */
    size_t flagSize;
    internal::readBytes(sizeof(Header), ptr, rest);
    if (!internal::size8(ptr, rest, flagSize) || flagSize != 8) {
        m_info.error = kInvalidFlagSizeError;
        return false;
    }
    info.codec = *reinterpret_cast<uint8_t *>(ptr) == 1 ? IString::kUTF8 : IString::kUTF16;
    info.additionalUVSize = *reinterpret_cast<uint8_t *>(ptr + 1);
    info.vertexIndexSize = *reinterpret_cast<uint8_t *>(ptr + 2);
    info.textureIndexSize = *reinterpret_cast<uint8_t *>(ptr + 3);
    info.materialIndexSize = *reinterpret_cast<uint8_t *>(ptr + 4);
    info.boneIndexSize = *reinterpret_cast<uint8_t *>(ptr + 5);
    info.morphIndexSize = *reinterpret_cast<uint8_t *>(ptr + 6);
    info.rigidBodyIndexSize = *reinterpret_cast<uint8_t *>(ptr + 7);
    internal::readBytes(flagSize, ptr, rest);

    /* name in Japanese */
    if (!internal::sizeText(ptr, rest, info.namePtr, info.nameSize)) {
        m_info.error = kInvalidNameSizeError;
        return false;
    }
    /* name in English */
    if (!internal::sizeText(ptr, rest, info.englishNamePtr, info.englishNameSize)) {
        m_info.error = kInvalidEnglishNameSizeError;
        return false;
    }
    /* comment in Japanese */
    if (!internal::sizeText(ptr, rest, info.commentPtr, info.commentSize)) {
        m_info.error = kInvalidCommentSizeError;
        return false;
    }
    /* comment in English */
    if (!internal::sizeText(ptr, rest, info.englishCommentPtr, info.englishCommentSize)) {
        m_info.error = kInvalidEnglishCommentSizeError;
        return false;
    }

    /* vertex */
    if (!Vertex::preparse(ptr, rest, info)) {
        m_info.error = kInvalidVerticesError;
        return false;
    }

    /* indices */
    size_t nindices;
    if (!internal::size32(ptr, rest, nindices) || nindices * info.vertexIndexSize > rest) {
        m_info.error = kInvalidIndicesError;
        return false;
    }
    info.indicesPtr = ptr;
    info.indicesCount = nindices;
    internal::readBytes(nindices * info.vertexIndexSize, ptr, rest);

    /* texture lookup table */
    size_t ntextures;
    if (!internal::size32(ptr, rest, ntextures)) {
        m_info.error = kInvalidTextureSizeError;
        return false;
    }
    info.texturesPtr = ptr;
    for (size_t i = 0; i < ntextures; i++) {
        size_t nNameSize;
        uint8_t *namePtr;
        if (!internal::sizeText(ptr, rest, namePtr, nNameSize)) {
            m_info.error = kInvalidTextureError;
            return false;
        }
    }
    info.texturesCount = ntextures;

    /* material */
    if (!Material::preparse(ptr, rest, info)) {
        m_info.error = kInvalidMaterialsError;
        return false;
    }

    /* bone */
    if (!Bone::preparse(ptr, rest, info)) {
        m_info.error = kInvalidBonesError;
        return false;
    }

    /* morph */
    if (!Morph::preparse(ptr, rest, info)) {
        m_info.error = kInvalidMorphsError;
        return false;
    }

    /* display name table */
    if (!Label::preparse(ptr, rest, info)) {
        m_info.error = kInvalidLabelsError;
        return false;
    }

    /* rigid body */
    if (!RigidBody::preparse(ptr, rest, info)) {
        m_info.error = kInvalidRigidBodiesError;
        return false;
    }

    /* constraint */
    if (!Joint::preparse(ptr, rest, info)) {
        m_info.error = kInvalidJointsError;
        return false;
    }
    info.endPtr = ptr;
    info.encoding = m_encodingRef;

    return rest == 0;
}

void Model::setVisible(bool value)
{
    m_visible = value;
}

Scalar Model::edgeScaleFactor(const Vector3 &cameraPosition) const
{
    Scalar length = 0;
    if (m_bones.count() > 1) {
        IBone *bone = m_bones.at(1);
        length = (cameraPosition - bone->worldTransform().getOrigin()).length() * m_edgeWidth;
    }
    return length / 1000.0f;
}

void Model::setName(const IString *value)
{
    internal::setString(value, m_name);
}

void Model::setEnglishName(const IString *value)
{
    internal::setString(value, m_englishName);
}

void Model::setComment(const IString *value)
{
    internal::setString(value, m_comment);
}

void Model::setEnglishComment(const IString *value)
{
    internal::setString(value, m_englishComment);
}

void Model::release()
{
    leaveWorld(m_worldRef);
    internal::zerofill(&m_info, sizeof(m_info));
    m_vertices.releaseAll();
    m_textures.releaseAll();
    m_materials.releaseAll();
    m_bones.releaseAll();
    m_morphs.releaseAll();
    m_labels.releaseAll();
    m_rigidBodies.releaseAll();
    m_joints.releaseAll();
    delete m_name;
    m_name = 0;
    delete m_englishName;
    m_englishName = 0;
    delete m_comment;
    m_comment = 0;
    delete m_englishComment;
    m_englishComment = 0;
    m_position.setZero();
    m_rotation.setValue(0, 0, 0, 1);
    m_opacity = 1;
    m_scaleFactor = 1;
}

void Model::parseNamesAndComments(const DataInfo &info)
{
    IEncoding *encoding = info.encoding;
    internal::setStringDirect(encoding->toString(info.namePtr, info.nameSize, info.codec), m_name);
    internal::setStringDirect(m_encodingRef->toString(info.englishNamePtr, info.englishNameSize, info.codec), m_englishName);
    internal::setStringDirect(m_encodingRef->toString(info.commentPtr, info.commentSize, info.codec), m_comment);
    internal::setStringDirect(m_encodingRef->toString(info.englishCommentPtr, info.englishCommentSize, info.codec), m_englishComment);
}

void Model::parseVertices(const DataInfo &info)
{
    const int nvertices = info.verticesCount;
    uint8_t *ptr = info.verticesPtr;
    size_t size;
    for(int i = 0; i < nvertices; i++) {
        Vertex *vertex = new Vertex();
        m_vertices.add(vertex);
        vertex->read(ptr, info, size);
        ptr += size;
    }
}

void Model::parseIndices(const DataInfo &info)
{
    const int nindices = info.indicesCount;
    const int nvertices = info.verticesCount;
    uint8_t *ptr = info.indicesPtr;
    size_t size = info.vertexIndexSize;
    for(int i = 0; i < nindices; i++) {
        int index = internal::readUnsignedIndex(ptr, size);
        if (index >= 0 && index < nvertices) {
            m_indices.add(index);
        }
        else {
            m_indices.add(0);
        }
    }
}

void Model::parseTextures(const DataInfo &info)
{
    const int ntextures = info.texturesCount;
    uint8_t *ptr = info.texturesPtr;
    uint8_t *texturePtr;
    size_t nTextureSize, rest = SIZE_MAX;
    for(int i = 0; i < ntextures; i++) {
        internal::sizeText(ptr, rest, texturePtr, nTextureSize);
        m_textures.add(m_encodingRef->toString(texturePtr, nTextureSize, info.codec));
    }
}

void Model::parseMaterials(const DataInfo &info)
{
    const int nmaterials = info.materialsCount;
    uint8_t *ptr = info.materialsPtr;
    size_t size;
    for(int i = 0; i < nmaterials; i++) {
        Material *material = new Material();
        m_materials.add(material);
        material->read(ptr, info, size);
        ptr += size;
    }
}

void Model::parseBones(const DataInfo &info)
{
    const int nbones = info.bonesCount;
    uint8_t *ptr = info.bonesPtr;
    size_t size;
    for(int i = 0; i < nbones; i++) {
        Bone *bone = new Bone();
        m_bones.add(bone);
        bone->read(ptr, info, size);
        bone->performTransform();
        bone->performUpdateLocalTransform();
        m_name2boneRefs.insert(bone->name()->toHashString(), bone);
        m_name2boneRefs.insert(bone->englishName()->toHashString(), bone);
        ptr += size;
    }
}

void Model::parseMorphs(const DataInfo &info)
{
    const int nmorphs = info.morphsCount;
    uint8_t *ptr = info.morphsPtr;
    size_t size;
    for(int i = 0; i < nmorphs; i++) {
        Morph *morph = new Morph();
        m_morphs.add(morph);
        morph->read(ptr, info, size);
        m_name2morphRefs.insert(morph->name()->toHashString(), morph);
        m_name2morphRefs.insert(morph->englishName()->toHashString(), morph);
        ptr += size;
    }
}

void Model::parseLabels(const DataInfo &info)
{
    const int nlabels = info.labelsCount;
    uint8_t *ptr = info.labelsPtr;
    size_t size;
    for(int i = 0; i < nlabels; i++) {
        Label *label = new Label();
        m_labels.add(label);
        label->read(ptr, info, size);
        ptr += size;
    }
}

void Model::parseRigidBodies(const DataInfo &info)
{
    const int nRigidBodies = info.rigidBodiesCount;
    uint8_t *ptr = info.rigidBodiesPtr;
    size_t size;
    for(int i = 0; i < nRigidBodies; i++) {
        RigidBody *rigidBody = new RigidBody();
        m_rigidBodies.add(rigidBody);
        rigidBody->read(ptr, info, size);
        ptr += size;
    }
}

void Model::parseJoints(const DataInfo &info)
{
    const int nJoints = info.jointsCount;
    uint8_t *ptr = info.jointsPtr;
    size_t size;
    for(int i = 0; i < nJoints; i++) {
        Joint *joint = new Joint();
        m_joints.add(joint);
        joint->read(ptr, info, size);
        ptr += size;
    }
}

void Model::getIndexBuffer(IIndexBuffer *&indexBuffer) const
{
    delete indexBuffer;
    indexBuffer = new IndexBuffer(m_indices, m_vertices.count());
}

void Model::getStaticVertexBuffer(IStaticVertexBuffer *&staticBuffer) const
{
    delete staticBuffer;
    staticBuffer = new StaticVertexBuffer(this);
}

void Model::getDynamicVertexBuffer(IDynamicVertexBuffer *&dynamicBuffer,
                                   const IIndexBuffer *indexBuffer) const
{
    delete dynamicBuffer;
    dynamicBuffer = new DynamicVertexBuffer(this, indexBuffer);
}

void Model::getMatrixBuffer(IMatrixBuffer *&matrixBuffer,
                            IDynamicVertexBuffer *dynamicBuffer,
                            const IIndexBuffer *indexBuffer) const
{
    delete matrixBuffer;
    matrixBuffer = new MatrixBuffer(this,
                                    static_cast<const IndexBuffer *>(indexBuffer),
                                    static_cast<DynamicVertexBuffer *>(dynamicBuffer));
}

}
}

#include "Archive.hh"

#include <common/Align.hh>
#include <common/Bytes.hh>
#include <common/Memory.hh>

extern "C" {
#include <string.h>
}

Archive::Tree::Tree(u8 *tree) : m_tree(tree) {}

Archive::Tree::~Tree() {}

bool Archive::Tree::isValid(u32 treeSize, u32 filesSize) const {
    if (!Memory::IsAligned(m_tree, 0x04)) {
        return false;
    }

    if (!isHeaderValid(treeSize)) {
        return false;
    }

    for (u32 i = 0; i < getDirCount(); i++) {
        if (!getDir(i).isValid(*this)) {
            return false;
        }
    }

    for (u32 i = 0; i < getNodeCount(); i++) {
        if (!getNode(i).isValid(*this, filesSize)) {
            return false;
        }
    }

    return true;
}

bool Archive::Tree::isHeaderValid(u32 treeSize) const {
    if (treeSize < 0x20) {
        return false;
    }

    u32 dirsOffset = Bytes::ReadBE<u32>(m_tree, 0x04);
    u32 dirsSize = Bytes::ReadBE<u32>(m_tree, 0x00) * 0x10;
    if (!IsAligned(dirsOffset, 0x4) || !IsAligned(dirsSize, 0x4)) {
        return false;
    }
    if (dirsOffset > treeSize || dirsSize > treeSize) {
        return false;
    }
    if (dirsOffset + dirsSize < dirsOffset || dirsOffset + dirsSize > treeSize) {
        return false;
    }

    u32 nodesOffset = Bytes::ReadBE<u32>(m_tree, 0x0c);
    u32 nodesSize = Bytes::ReadBE<u32>(m_tree, 0x08) * 0x14;
    if (!IsAligned(nodesOffset, 0x4) || !IsAligned(nodesSize, 0x4)) {
        return false;
    }
    if (nodesOffset > treeSize || nodesSize > treeSize) {
        return false;
    }
    if (nodesOffset + nodesSize < nodesOffset || nodesOffset + nodesSize > treeSize) {
        return false;
    }

    u32 namesOffset = Bytes::ReadBE<u32>(m_tree, 0x14);
    u32 namesSize = Bytes::ReadBE<u32>(m_tree, 0x10);
    if (!IsAligned(namesOffset, 0x4) || !IsAligned(namesSize, 0x4)) {
        return false;
    }
    if (namesOffset > treeSize || namesSize > treeSize) {
        return false;
    }
    if (namesOffset + nodesSize < namesOffset || namesOffset + namesSize > treeSize) {
        return false;
    }

    if (nodesOffset < dirsOffset || namesOffset < nodesOffset) {
        return false;
    }
    if (dirsOffset + dirsSize > nodesOffset || nodesOffset + nodesSize > namesOffset) {
        return false;
    }

    if (namesSize < 1 || getNames()[namesSize - 1] != '\0') {
        return false;
    }

    return true;
}

u8 *Archive::Tree::get() const {
    return m_tree;
}

Archive::Dir Archive::Tree::getDir(u32 index) const {
    return Dir(m_tree + Bytes::ReadBE<u32>(m_tree, 0x04) + index * 0x10);
}

u32 Archive::Tree::getDirCount() const {
    return Bytes::ReadBE<u32>(m_tree, 0x00);
}

Archive::Node Archive::Tree::getNode(u32 index) const {
    return Node(m_tree + Bytes::ReadBE<u32>(m_tree, 0x0c) + index * 0x14);
}

u32 Archive::Tree::getNodeCount() const {
    return Bytes::ReadBE<u32>(m_tree, 0x08);
}

char *Archive::Tree::getNames() const {
    return reinterpret_cast<char *>(m_tree + Bytes::ReadBE<u32>(m_tree, 0x14));
}

u32 Archive::Tree::getNamesSize() const {
    return Bytes::ReadBE<u32>(m_tree, 0x10);
}

bool Archive::Tree::search(const char *path, const char *&name, Dir &dir, Node &node,
        bool &exists) const {
    dir = getDir(0);
    name = path + 1;
    for (const char *nextName; *name; name = nextName) {
        nextName = strchr(name, '/');
        u32 nameLength = nextName ? nextName - name : strlen(name);
        nextName = nextName ? nextName + 1 : name + nameLength;

        u32 i;
        for (i = 0; i < dir.getNodeCount(); i++) {
            node = dir.getNode(i, *this);
            if (!strncmp(node.getName(getNames()), name, nameLength)) {
                break;
            }
        }
        if (i == dir.getNodeCount()) {
            exists = false;
            node = dir.getNode(dir.getNodeCount(), *this);
            return !*nextName;
        }

        if (node.isDir()) {
            dir = node.getDir(*this);
        } else {
            if (*nextName) {
                return false;
            }
        }
    }
    exists = true;
    return true;
}

Archive::Dir::Dir(u8 *dir) : m_dir(dir) {}

Archive::Dir::~Dir() {}

bool Archive::Dir::isValid(Tree tree) const {
    if (!Memory::IsAligned(m_dir, 0x04)) {
        return false;
    }

    if (Bytes::ReadBE<u32>(m_dir, 0x04) >= tree.getNamesSize() - 1) {
        return false;
    }

    u32 firstNode = Bytes::ReadBE<u32>(m_dir, 0x0c);
    u16 nodeCount = getNodeCount();
    if (firstNode > tree.getNodeCount() || nodeCount > tree.getNodeCount()) {
        return false;
    }
    if (firstNode + nodeCount < firstNode || firstNode + nodeCount > tree.getNodeCount()) {
        return false;
    }

    return true;
}

u8 *Archive::Dir::get() const {
    return m_dir;
}

u32 Archive::Dir::getType() const {
    return Bytes::ReadBE<u32>(m_dir, 0x00);
}

char *Archive::Dir::getName(char *names) const {
    return names + Bytes::ReadBE<u32>(m_dir, 0x04);
}

u16 Archive::Dir::getNameHash() const {
    return Bytes::ReadBE<u16>(m_dir, 0x08);
}

u16 Archive::Dir::getNodeCount() const {
    return Bytes::ReadBE<u16>(m_dir, 0x0a);
}

Archive::Node Archive::Dir::getNode(u32 index, Tree tree) const {
    return tree.getNode(Bytes::ReadBE<u32>(m_dir, 0x0c) + index);
}

Archive::Node::Node(u8 *node) : m_node(node) {}

Archive::Node::~Node() {}

bool Archive::Node::isValid(Tree tree, u32 filesSize) const {
    if (!Memory::IsAligned(m_node, 0x04)) {
        return false;
    }

    if (!isDir() && !isFile()) {
        return false;
    }

    if ((Bytes::ReadBE<u32>(m_node, 0x04) & 0xffffff) >= tree.getNamesSize() - 1) {
        return false;
    }

    if (isDir()) {
        if (Bytes::ReadBE<u32>(m_node, 0x08) >= tree.getDirCount()) {
            return false;
        }
    }

    if (isFile()) {
        u32 fileOffset = Bytes::ReadBE<u32>(m_node, 0x08);
        u32 fileSize = getFileSize();
        if (!IsAligned(fileOffset, 0x20)) {
            return false;
        }
        if (fileOffset > filesSize || fileSize > filesSize) {
            return false;
        }
        if (fileOffset + fileSize < fileOffset || fileOffset + fileSize > filesSize) {
            return false;
        }
    }

    return true;
}

u8 *Archive::Node::get() const {
    return m_node;
}

u16 Archive::Node::getFileIndex() const {
    return Bytes::ReadBE<u16>(m_node, 0x00);
}

u16 Archive::Node::getNameHash() const {
    return Bytes::ReadBE<u16>(m_node, 0x02);
}

bool Archive::Node::isDir() const {
    return Bytes::ReadBE<u8>(m_node, 0x04) & 0x2;
}

bool Archive::Node::isFile() const {
    return Bytes::ReadBE<u8>(m_node, 0x04) & 0x1;
}

char *Archive::Node::getName(char *names) const {
    return reinterpret_cast<char *>(names + (Bytes::ReadBE<u32>(m_node, 0x04) & 0xffffff));
}

Archive::Dir Archive::Node::getDir(Tree tree) const {
    return tree.getDir(Bytes::ReadBE<u32>(m_node, 0x08));
}

void *Archive::Node::getFile(u8 *files) const {
    return files + Bytes::ReadBE<u32>(m_node, 0x08);
}

u32 Archive::Node::getFileSize() const {
    return Bytes::ReadBE<u32>(m_node, 0x0c);
}

Archive::Archive(u8 *archive) : m_archive(archive) {}

Archive::~Archive() {}

bool Archive::isValid(u32 archiveSize) const {
    if (!Memory::IsAligned(m_archive, 0x20)) {
        return false;
    }

    if (!isHeaderValid(archiveSize)) {
        return false;
    }

    if (!getTree().isValid(getTreeSize(), getFilesSize())) {
        return false;
    }

    return true;
}

bool Archive::isHeaderValid(u32 archiveSize) const {
    if (archiveSize < 0x20) {
        return false;
    }

    if (memcmp(m_archive, "RARC", 0x04)) {
        return false;
    }

    if (Bytes::ReadBE<u32>(m_archive, 0x04) != archiveSize) {
        return false;
    }

    u32 treeOffset = Bytes::ReadBE<u32>(m_archive, 0x08);
    u32 treeSize = Bytes::ReadBE<u32>(m_archive, 0x0c);
    if (!IsAligned(treeOffset, 0x20) || !IsAligned(treeSize, 0x20)) {
        return false;
    }
    if (treeOffset > archiveSize || treeSize > archiveSize) {
        return false;
    }
    if (treeOffset + treeSize < treeOffset || treeOffset + treeSize > archiveSize) {
        return false;
    }

    if (Bytes::ReadBE<u32>(m_archive, 0x10) != archiveSize - (treeOffset + treeSize)) {
        return false;
    }
    if (Bytes::ReadBE<u32>(m_archive, 0x14) != archiveSize - (treeOffset + treeSize)) {
        return false;
    }

    return true;
}

u8 *Archive::get() const {
    return m_archive;
}

Archive::Tree Archive::getTree() const {
    return Tree(m_archive + Bytes::ReadBE<u32>(m_archive, 0x08));
}

u32 Archive::getTreeSize() const {
    return Bytes::ReadBE<u32>(m_archive, 0x0c);
}

u8 *Archive::getFiles() const {
    return m_archive + Bytes::ReadBE<u32>(m_archive, 0x08) + Bytes::ReadBE<u32>(m_archive, 0x0c);
}

u32 Archive::getFilesSize() const {
    return Bytes::ReadBE<u32>(m_archive, 0x10);
}

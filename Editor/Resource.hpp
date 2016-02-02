#ifndef URDE_RESOURCE_HPP
#define URDE_RESOURCE_HPP

#include <HECL/Database.hpp>
#include "Space.hpp"

namespace URDE
{

/** Combines a ProjectPath with actively used Space references
 *
 *  This class is intended to be heap-allocated in a hierarchical mapping, so the entire tree
 *  of resources is available in-memory to systems that need it. Refreshes of the index will
 *  continue to use existing allocations that haven't been deleted.
 *
 *  The key purpose of this class is to centrally register observer-nodes for resources that
 *  are updated via editing, or external file changes.
 */
class Resource
{
public:
    using ProjectDataSpec = HECL::Database::Project::ProjectDataSpec;
private:
    HECL::ProjectPath m_path;
    Space::Class m_defaultClass = Space::Class::None;
    EditorSpace* m_editingSpace = nullptr;
    std::vector<ViewerSpace*> m_viewingSpaces;
public:
    static Space::Class DeduceDefaultSpaceClass(const HECL::ProjectPath& path);
    explicit Resource(HECL::ProjectPath&& path)
    : m_path(std::move(path)), m_defaultClass(DeduceDefaultSpaceClass(m_path)) {}
    const HECL::ProjectPath& path() const {return m_path;}
};

/** Provides centralized hierarchical lookup and ownership of Resource nodes */
class ResourceTree
{
public:
    struct Node
    {
        std::map<HECL::ProjectPath, std::unique_ptr<Node>> m_subnodes;
        std::map<HECL::ProjectPath, std::unique_ptr<Resource>> m_resources;
    };
private:
    std::unique_ptr<Node> m_rootNode;
};

}

#endif // URDE_RESOURCE_HPP
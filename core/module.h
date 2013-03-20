#ifndef MODULE_H
#define MODULE_H

#include <list>
#include <map>
#include <set>
#include <algorithm>
#include <memory>

#include "objecttype.h"
#include "objecttypetemplate.h"
#include "standardformatdetector.h"

class Object;
class Parser;

/*!
 * \brief The Module class
 */
class Module
{
public:
    Module();

    void import(const Module& module);

    bool isExtension(const ObjectType& child, const ObjectType& parent) const;
    void setExtension(const ObjectTypeTemplate& childTemplate, const std::function<ObjectType(const ObjectType &)> &parentFunction);
    void setExtension(const ObjectTypeTemplate& childTemplate, const ObjectType& parent);
    void setExtension(const ObjectTypeTemplate& childTemplate, const ObjectType& parent, const std::map<int, int> &bindings);
    ObjectType getFather(const ObjectType& child) const;

    ObjectType specify(const ObjectType& parent) const;
    void setSpecification(const ObjectType& parent, const ObjectType& child);

    Object* handle(const ObjectType& type, File& file, Object* parent = nullptr) const;
    bool  canHandle(const ObjectType& type) const;
    const Module* handler(const ObjectType &type) const;

    const ObjectTypeTemplate& getTemplate(const std::string& name) const;
    bool hasTemplate(const std::string& name) const;
    int64_t getFixedSize(const ObjectType& type) const;

protected:
    virtual void addFormatDetection(StandardFormatDetector::Adder& formatAdder);
    virtual void requestImportations(std::vector<std::string>& formatRequested);
    virtual bool doLoad();


    virtual Parser* getParser(const ObjectType &type, Object& data, const Module& fromModule) const;
    virtual bool hasParser(const ObjectType &type) const;
    virtual int64_t doGetFixedSize(const ObjectType& type, const Module& module) const;

    void addTemplate(const ObjectTypeTemplate& typeTemplate);
    const ObjectTypeTemplate& newTemplate(const std::string& name);
    const ObjectTypeTemplate& newTemplate(const std::string& name, const std::vector<std::string>& parameters);

private:
    friend class ModuleLoader;
    template<class T>
    struct UnrefCompare : public std::binary_function<T, T, bool>
    {
        bool operator()(T x, T y) const
        {
            return *x < *y;
        }
    };

    typedef std::map<const ObjectType*, ObjectType, UnrefCompare<const ObjectType*> > SpecificationMap;

    typedef std::map<const ObjectTypeTemplate*, std::function<ObjectType(const ObjectType&)>, UnrefCompare<const ObjectTypeTemplate*> > ExtensionMap;

    bool load();

    ObjectType specifyLocally(const ObjectType& parent) const;
    void addParsers(Object& data, const ObjectType &type, const Module& fromModule, const ObjectType &lastType = ObjectType()) const;

    Object* handle(const ObjectType& type, File& file, Object *parent, const Module& fromModule) const;

    bool _loaded;

    std::list<const Module*> _importedModules;

    ExtensionMap _extensions;

    SpecificationMap _automaticSpecifications;

    std::map<std::string, const ObjectTypeTemplate*> _templates;
    std::list<std::unique_ptr<ObjectTypeTemplate> > _ownedTemplates;
};

#endif // MODULE_H

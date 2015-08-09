//This file is part of the HexaMonkey project, a multimedia analyser
//Copyright (C) 2013  Sevan Drapeau-Martin, Nicolas Fleury

//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either version 2
//of the License, or (at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#ifndef MODULE_H
#define MODULE_H

#define HM_UNKNOWN_SIZE -1
#define HM_PARENT_SIZE -2

#include <list>
#include <map>
#include <set>
#include <algorithm>
#include <memory>

#include "core/objecttype.h"
#include "core/objecttypetemplate.h"
#include "core/formatdetector/standardformatdetector.h"
#include "core/variable/variable.h"

class Object;
class Parser;
class Variable;

/*!
 * @brief Factory for \link Object objects\endlink
 *
 * Given a \link ObjectType type\endlink and a \link File file\endlink it can create an object beginning at the current position
 * of the \link File file\endlink and add the appropriate \link Parser parsers\endlink.
 *
 * It holds an object model that defines extensions and specification for \link ObjectType types\endlink handled. This will define which
 * parsers are added. Each parser correspond to a \link ObjectType type\endlink generated by getParser, if the \link ObjectType type\endlink A extends
 * the \link ObjectType type\endlink B then the parser for B will be added and then the parser for A.
 *
 * In addition to extensions, the object model defines specification, which means that if after minimal parser an \link Object object\endlink has
 * \link ObjectType type\endlink A and A's specification is B then \link Parser parser\endlink for B will be subsequently added and the
 * \link ObjectType type\endlink of the \link Object object\endlink will be B.
 *
 * It can also define functions that can be used in HMDLs.
 *
 * Other modules can be imported, in which case if a \link ObjectType type\endlink or a function can not be handle by the \link Module module\endlink,
 * then the imported \link Module modules\endlink will serve as fallback with priority for the last loaded.
 *
 * The life cycle of the \link Module module\endlink is handled by a \link ModuleLoader module loader\endlink : on construction the \link Module module\endlink must
 * have the smallest memory footprint possible abd the shortest runtime, when added to the \link ModuleLoader module loader\endlink format detection will be added so
 * that the module loader knows on which file the \link Module module\endlink should be used (reimplement addFormatDetection). Then when when the \link ModuleLoader
 * module loader\endlink uses the module for the first time, it first ask for importation request to see which \link Module modules\endlink must be loaded and imported
 * (reimplement requestImportations) and then the actual loading is done (reimplement doLoad).
 *
 * The class is virtual and must be subclassed to be used. For most usages MapModule is the right class to subclass.
 */
class Module
{
public:
    Module();

    /** @brief Import a \link Module module\endlink to serve as fallback if a \link ObjectType type\endlink cannot be handled directly by the
     *  \link Module module\endlink
     *
     * If a \link ObjectType type\endlink or a function is not handled by the the \link Module module\endlink, then a handler will be searched among the
     * imported modules, with a priority for the lastest \link Module module\endlink imported
     */
    void import(const Module& module);

    /**
     * @brief Check if a \link ObjectType type\endlink extends another
     *
     * Get the father of the child and checks if it \link ObjectType::extendsDirectly extends directly\endlink the parent, if not checks recursively
     * if the father extends the parent.
     */
    bool isExtension(const ObjectType& child, const ObjectType& parent) const;

    /**
     * @brief Set a function to compute the fathers for a \link ObjectTypeTemplate type template\endlink
     *
     * Most likely the fathers will all have the same \link ObjectTypeTemplate type template\endlink but different parameters
     */
    void setExtension(const ObjectTypeTemplate& childTemplate, const std::function<ObjectType(const ObjectType &)> &parentFunction);

    /**
     * @brief Set a father for a \link ObjectTypeTemplate type template\endlink
     */
    void setExtension(const ObjectTypeTemplate& childTemplate, const ObjectType& parent);

    /**
     * @brief Set fathers for a \link ObjectTypeTemplate type template\endlink with a mapping for parameters
     */
    void setExtension(const ObjectTypeTemplate& childTemplate, const ObjectType& parent, const std::map<int, int> &parameterMapping);

    /**
     * @brief Get the father of a \link ObjectType type\endlink
     *
     * Returns the \link ObjectType::isNull null\endlink type if the \link ObjectType type\endlink doesn't have a father
     */
    ObjectType getFather(const ObjectType& child) const;

    /**
     * @brief Get the specification for the parent object
     *
     * Returns the \link ObjectType::isNull null\endlink type if the \link ObjectType type\endlink doesn't have a specification
     */
    ObjectType specify(const ObjectType& parent) const;

    /**
     * @brief Set the specification for the parent
     */
    void setSpecification(const ObjectType& parent, const ObjectType& child);


    Object* handleFile(const ObjectType &type, File& file, VariableCollector& collector) const;
    /**
     * @brief Create an object beginning at the current position of the file and add the appropriate \link Parser
     * parsers\endlink according to the inheritance structure for the type
     */
    Object* handle(const ObjectType& type, Object& parent) const;

    /**
     * @brief Check if the type can be handled by the \link Module module\endlink or any of the imported one.
     */
    bool  canHandle(const ObjectType& type) const;

    /**
     * @brief Get the module able to handle the type among the current \link Module module\endlink and the imported ones
     *
     * Return nullptr if the type cannot be handled
     */
    const Module* handler(const ObjectType &type) const;

    /**
     * @brief Get the size that every objects of a type should have
     *
     * Return -1 if the size is variable
     */
    int64_t getFixedSize(const ObjectType& type) const;

    /**
     * @brief Get a \link ObjectTypeTemplate type template\endlink stored by the \link Module module\endlink or one of the imported one, by its name
     */
    const ObjectTypeTemplate& getTemplate(const std::string& name) const;

    /**
     * @brief Check if any \link ObjectTypeTemplate type template\endlink stored by the \link Module module\endlink or one of the imported one has this name
     */
    bool hasTemplate(const std::string& name) const;

    /**
     * @brief Check if the \link Module module\endlink or any of the imported ones can handle a function by this name
     */
    bool canHandleFunction(const std::string &name) const;

    /**
     * @brief Get the module able to handle a function by this name among the current\link Module module\endlink and the imported ones
     *
     * Return nullptr if no function by this name can be handled
     */
    const Module* functionHandler(const std::string &name) const;

    /**
     * @brief Execute the function with the parameters given as a \link Scope scope\endlink
     */
    Variable executeFunction(const std::string &name, const Variable &params) const;

    /**
     * @brief Get the names of the parameters as a vector of strings
     */
    const std::vector<std::string>& getFunctionParameterNames(const std::string& name) const;

    /**
     * @brief Get a vector of boolean specifying for each parameter if the function can modify the value or not
     */
    const std::vector<bool>& getFunctionParameterModifiables(const std::string& name) const;

    /**
     * @brief Get a vector of values specifying default values for each parameter
     */
    const std::vector<Variant>& getFunctionParameterDefaults(const std::string& name) const;

    /**
     * @brief Check if the module has been successfully loaded by the
     * \link ModuleLoader module loader\endlink
     */
    bool isLoaded() const;

protected:
    /**
     * @brief [Pure Virtual] Use the \link StandardFormatDetector::Adder format adder\endlink to add format detection methods, so that the
     * \link ModuleLoader module loader\endlink can choose which module to use to parse a file
     */
    virtual void addFormatDetection(StandardFormatDetector::Adder& formatAdder);

    /**
     * @brief [Pure Virtual] Complete the list of \link Module modules\endlink that the \link ModuleLoader module loader\endlink
     * should load and import to the module, by adding the names of the \link Module modules\endlink requested.
     */
    virtual void requestImportations(std::vector<std::string>& modulesRequested);

    /**
     * @brief [Pure Virtual] Initialize the module. The function will be called by the \link ModuleLoader module loader\endlink when the
     * module is first used, after the requested modules have been added.
     */
    virtual bool doLoad();

    /**
     * @brief [Pure Virtual] Get the parser associated with the type.
     *
     * Return nullptr if the type cannot be handled
     */
    virtual Parser* getParser(const ObjectType &type, Object& object, const Module& fromModule) const;

    /**
     * @brief [Pure Virtual] Check if the type can be handled be the \link Module module\endlink directly.
     */
    virtual bool hasParser(const ObjectType &type) const;

    /**
     * @brief [Pure Virtual] Get the size that every objects of a type should have.
     *
     * Return 0 if the type relies on its father to compute the size
     *
     * Return -1 if the size is variable and set by type's \link Parser parser\endlink
     */
    virtual int64_t doGetFixedSize(const ObjectType& type, const Module& module) const;


    virtual bool doCanHandleFunction(const std::string& name) const;
    virtual Variable doExecuteFunction(const std::string& name, const Variable &params, const Module &fromModule) const;
    virtual const std::vector<std::string>& doGetFunctionParameterNames(const std::string& name) const;
    virtual const std::vector<bool>& doGetFunctionParameterModifiables(const std::string& name) const;
    virtual const std::vector<Variant>& doGetFunctionParameterDefaults(const std::string& name) const;

    /**
     * @brief Register a \link ObjectTypeTemplate type template\endlink to the \link Module module\endlink so that it can be
     * accessed by its name by the function getTemplate.
     *
     * Every \link ObjectTypeTemplate type template\endlink should be registered so that when the \link Module module\endlink
     * is imported into an \link FromFileModule HMDL module\endlink, the \link ObjectTypeTemplate type template\endlink
     * can be recognized.
     */
    void addTemplate(const ObjectTypeTemplate& typeTemplate);

    /**
     * @brief
     */
    ObjectTypeTemplate& newTemplate(const std::string& name, const std::vector<std::string>& parameters = std::vector<std::string>());

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
    void addParsersRecursive(Object& object, const ObjectType &type, const Module& fromModule, const ObjectType &lastType) const;

    Object* handle(const ObjectType& type, File& file, Object *parent, VariableCollector& collector, const Module& fromModule) const;

    Variable executeFunction(const std::string& name, const Variable &params, const Module& fromModule) const;

    bool _loaded;

    std::list<const Module*> _importedModules;

    ExtensionMap _extensions;

    SpecificationMap _automaticSpecifications;

    std::unordered_map<std::string, const ObjectTypeTemplate*> _templates;
    std::list<std::unique_ptr<ObjectTypeTemplate> > _ownedTemplates;
};

#endif // MODULE_H

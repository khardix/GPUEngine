#ifndef GE_RG_ATTRIB_CONFIG_H
#define GE_RG_ATTRIB_CONFIG_H

#include <list>
#include <map>
#include <memory>
#include <vector>
#include <geRG/Export.h>
#include <geRG/AttribType.h>


namespace ge
{
   namespace rg
   {
      class AttribConfigRef;
      class AttribStorage;
      class Mesh;
      class RenderingContext;
      typedef uint16_t AttribConfigId;  ///< \brief Integer-based type for the most frequently used attribute configurations.


      class GERG_EXPORT AttribConfig {
      public:

         /** ConfigData holds complete information about attribute configuration.
          *
          *  It includes types of all active attributes, whether Element Buffer Object is used
          *  and id for quick ConfigData comparison and lookup. */
         struct ConfigData {
            std::vector<AttribType> attribTypes;  ///< Type of each active attribute. \sa AttribType
            bool ebo;                             ///< Indicates whether element buffer object is used or not.
            AttribConfigId id;                    ///< \brief Id of one of frequently used attribute configurations.
            inline ConfigData();  ///< Default constructor. Does nothing. The user is responsible for the object initialization.
            inline ConfigData(const std::vector<AttribType>& attribTypes,bool ebo);  ///< \brief Constructs object from parameters.
            inline ConfigData(const std::vector<AttribType>& attribTypes,bool ebo,AttribConfigId id);  ///< \brief Construct object from parameters.
            inline ConfigData(const ConfigData &data);  ///< Copy constructor.
            inline void updateId();  ///< \brief Updates id member of the object.
            inline bool operator==(const ConfigData &rhs) const;  ///< Comparison operator.
            inline bool operator!=(const ConfigData &rhs) const;  ///< Comparison operator.
            inline bool operator<=(const ConfigData &rhs) const;  ///< Comparison operator.
            inline bool operator>=(const ConfigData &rhs) const;  ///< Comparison operator.
            inline bool operator< (const ConfigData &rhs) const;  ///< Comparison operator.
            inline bool operator> (const ConfigData &rhs) const;  ///< Comparison operator.
         };
         typedef std::map<ConfigData,AttribConfig*> AttribConfigList;  ///< Map of AttribConfigs used inside RenderingContext class.
         typedef std::list<std::shared_ptr<AttribStorage>> AttribStorageList;  ///< List of AttribStorages used inside AttribConfig class.

      protected:

         int _referenceCounter = 0;
         ConfigData _configData;
         RenderingContext *_renderingContext;
         AttribConfigList::iterator _selfIterator;
         AttribStorageList _attribStorages; // private storages are on the end
         int _defaultStorageNumVertices = 1000*1024; // 1M vertices (for just float coordinates ~12MiB, including normals, color and texCoord, ~36MiB)
         int _defaultStorageNumIndices = 4000*1024; // 4M indices (~16MiB)
         int _defaultStorageNumDrawCommands = 100*1024; // 10 vertices per draw command, 100K draw commands (~1.6MiB)

         inline AttribConfig(const ConfigData &config,RenderingContext *rc,AttribConfigList::iterator selfIterator);
         inline AttribConfig(const std::vector<AttribType>& attribTypes,bool ebo,
                             RenderingContext *rc,AttribConfigList::iterator selfIterator);
         inline AttribConfig(const std::vector<AttribType>& attribTypes,bool ebo,
                             AttribConfigId id,RenderingContext *rc,AttribConfigList::iterator selfIterator);
         virtual void destroy();

      public:

         AttribConfig() = delete;
         virtual ~AttribConfig();
         virtual void detachFromRenderingContext();
         virtual void deleteAllAttribStorages();

         inline const AttribStorageList& getAttribStorageList() const;

         virtual bool allocData(Mesh &mesh,int numVertices,int numIndices,int numDrawCommands);
         virtual bool reallocData(Mesh &mesh,int numVertices,int numIndices,
                                  int numDrawCommands,bool preserveContent=true);
         inline void freeData(Mesh &mesh);

         inline AttribConfigRef createReference();
         virtual AttribConfigRef getOrCreate(const ConfigData &config,RenderingContext *rc);
         inline AttribConfigRef getOrCreate(const std::vector<AttribType>& attribTypes,bool ebo,
                                            RenderingContext *rc);
         inline AttribConfigRef getOrCreate(const std::vector<AttribType>& attribTypes,bool ebo,
                                            AttribConfigId id,RenderingContext *rc);

         inline void addReference();
         inline void removeReference();
         inline int referenceCounter();

         static AttribConfigId getId(const std::vector<AttribType>& attribTypes,bool ebo);

         inline const ConfigData& configData() const;
         inline RenderingContext* renderingContext() const;

         inline int defaultStorageNumVertices() const;
         inline void setDefaultStorageNumVertices(int num);
         inline int defaultStorageNumIndices() const;
         inline void setDefaultStorageNumIndices(int num);
         inline int defaultStorageNumDrawCommands() const;
         inline void setDefaultStorageNumDrawCommands(int num);

         inline bool operator==(const AttribConfig &rhs) const;
         inline bool operator!=(const AttribConfig &rhs) const;
         inline bool operator<=(const AttribConfig &rhs) const;
         inline bool operator>=(const AttribConfig &rhs) const;
         inline bool operator<(const AttribConfig &rhs) const;
         inline bool operator>(const AttribConfig &rhs) const;


         class Factory {
         public:
            inline AttribConfig* create(const ConfigData &config,RenderingContext *rc,
                                        AttribConfigList::iterator selfIterator);
            inline AttribConfig* create(const std::vector<AttribType>& attribTypes,bool ebo,
                                        RenderingContext *rc,AttribConfigList::iterator selfIterator);
            virtual AttribConfig* create(const std::vector<AttribType>& attribTypes,bool ebo,
                                         AttribConfigId id,RenderingContext *rc,
                                         AttribConfigList::iterator selfIterator);
         };
         static inline std::shared_ptr<Factory>& factory();
         static inline void setFactory(std::shared_ptr<Factory>& f);

      protected:
         static std::shared_ptr<Factory> _factory;
      };


      class GERG_EXPORT AttribConfigRef {
      protected:
         AttribConfig* _pointer;
      public:
         inline AttribConfig& get() const;
         inline AttribConfig& operator*() const;
         inline AttribConfig* operator->() const;
         inline AttribConfigRef();
         inline AttribConfigRef(AttribConfig &ac);
         inline AttribConfigRef(const AttribConfigRef &acr);
         inline AttribConfigRef(AttribConfigRef &&acr);
         inline AttribConfigRef(const AttribConfig::ConfigData &config);
         inline AttribConfigRef(const AttribConfig::ConfigData &config,RenderingContext *rc);
         inline AttribConfigRef(const std::vector<AttribType>& attribTypes,bool ebo);
         inline AttribConfigRef(const std::vector<AttribType>& attribTypes,bool ebo,AttribConfigId id);
         inline AttribConfigRef(const std::vector<AttribType>& attribTypes,bool ebo,RenderingContext *rc);
         inline AttribConfigRef(const std::vector<AttribType>& attribTypes,bool ebo,
                                AttribConfigId id,RenderingContext *rc);
         inline ~AttribConfigRef();
         inline AttribConfigRef& operator=(const AttribConfigRef &ac);
         inline AttribConfigRef& operator=(AttribConfigRef &&ac);
         inline bool operator==(const AttribConfigRef &rhs) const;
         inline bool operator!=(const AttribConfigRef &rhs) const;
         inline bool operator<=(const AttribConfigRef &rhs) const;
         inline bool operator>=(const AttribConfigRef &rhs) const;
         inline bool operator<(const AttribConfigRef &rhs) const;
         inline bool operator>(const AttribConfigRef &rhs) const;

         static const AttribConfigRef invalid;
      };

   }
}



// inline methods
#include <geRG/AttribStorage.h>
#include <geRG/RenderingContext.h>

namespace ge
{
   namespace rg
   {
      inline AttribConfig::ConfigData::ConfigData()  {}
      inline AttribConfig::ConfigData::ConfigData(const std::vector<AttribType>& attribTypes,bool ebo)
         : ConfigData(attribTypes,ebo,AttribConfig::getId(attribTypes,ebo))  {}
      inline AttribConfig::ConfigData::ConfigData(const std::vector<AttribType>& a,bool e,AttribConfigId i)
         : attribTypes(a), ebo(e), id(i)  {}
      inline AttribConfig::ConfigData::ConfigData(const ConfigData &data)
         : attribTypes(data.attribTypes), ebo(data.ebo), id(data.id)  {}
      inline void AttribConfig::ConfigData::updateId()  { id=AttribConfig::getId(attribTypes,ebo); }
      inline bool AttribConfig::ConfigData::operator==(const ConfigData &rhs) const  { return id!=0||rhs.id!=0 ? id==rhs.id : ebo==rhs.ebo&&attribTypes==rhs.attribTypes; }
      inline bool AttribConfig::ConfigData::operator!=(const ConfigData &rhs) const  { return id!=0||rhs.id!=0 ? id!=rhs.id : ebo!=rhs.ebo||attribTypes!=rhs.attribTypes; }
      inline bool AttribConfig::ConfigData::operator<=(const ConfigData &rhs) const  { return id!=0||rhs.id!=0 ? id<=rhs.id : ebo!=rhs.ebo?ebo<rhs.ebo:attribTypes<=rhs.attribTypes; }
      inline bool AttribConfig::ConfigData::operator>=(const ConfigData &rhs) const  { return id!=0||rhs.id!=0 ? id>=rhs.id : ebo!=rhs.ebo?ebo>rhs.ebo:attribTypes>=rhs.attribTypes; }
      inline bool AttribConfig::ConfigData::operator<(const ConfigData &rhs) const  { return id!=0||rhs.id!=0 ? id<rhs.id : ebo!=rhs.ebo?ebo<rhs.ebo:attribTypes<rhs.attribTypes; }
      inline bool AttribConfig::ConfigData::operator>(const ConfigData &rhs) const  { return id!=0||rhs.id!=0 ? id>rhs.id : ebo!=rhs.ebo?ebo>rhs.ebo:attribTypes>rhs.attribTypes; }

      inline AttribConfig::AttribConfig(const ConfigData &config,RenderingContext *rc,
         AttribConfigList::iterator selfIterator)
         : _configData(config), _renderingContext(rc), _selfIterator(selfIterator)  {}
      inline AttribConfig::AttribConfig(const std::vector<AttribType>& attribTypes,bool ebo,
         RenderingContext *rc,AttribConfigList::iterator selfIterator)
         : _configData(attribTypes,ebo,getId(attribTypes,ebo))
         , _renderingContext(rc), _selfIterator(selfIterator)  {}
      inline AttribConfig::AttribConfig(const std::vector<AttribType>& attribTypes,bool ebo,
         AttribConfigId id,RenderingContext *rc,AttribConfigList::iterator selfIterator)
         : _configData(attribTypes,ebo,id), _renderingContext(rc), _selfIterator(selfIterator)  {}
      const AttribConfig::AttribStorageList& AttribConfig::getAttribStorageList() const  { return _attribStorages; }
      inline void AttribConfig::freeData(Mesh &mesh)  { if(mesh.attribStorage()) mesh.attribStorage()->freeData(mesh); }
      inline AttribConfigRef AttribConfig::createReference()  { return AttribConfigRef(*this); }
      inline AttribConfigRef AttribConfig::getOrCreate(const ConfigData &config,RenderingContext *rc)
      { return rc->getAttribConfig(config); }
      inline AttribConfigRef AttribConfig::getOrCreate(const std::vector<AttribType>& attribTypes,bool ebo,
         RenderingContext *rc)  { return getOrCreate(attribTypes,ebo,getId(attribTypes,ebo),rc); }
      inline AttribConfigRef AttribConfig::getOrCreate(const std::vector<AttribType>& attribTypes,bool ebo,
         AttribConfigId id,RenderingContext *rc)  { return rc->getAttribConfig(attribTypes,ebo,id); }
      inline void AttribConfig::addReference()  { _referenceCounter++; }
      inline void AttribConfig::removeReference()  { if(--_referenceCounter==0) destroy(); }
      inline int AttribConfig::referenceCounter()  { return _referenceCounter; }
      inline const AttribConfig::ConfigData& AttribConfig::configData() const  { return _configData; }
      inline RenderingContext* AttribConfig::renderingContext() const  { return _renderingContext; }
      inline int AttribConfig::defaultStorageNumVertices() const  { return _defaultStorageNumVertices; }
      inline void AttribConfig::setDefaultStorageNumVertices(int num)  { _defaultStorageNumVertices=num; }
      inline int AttribConfig::defaultStorageNumIndices() const  { return _defaultStorageNumIndices; }
      inline void AttribConfig::setDefaultStorageNumIndices(int num)  { _defaultStorageNumIndices=num; }
      inline int AttribConfig::defaultStorageNumDrawCommands() const  { return _defaultStorageNumDrawCommands; }
      inline void AttribConfig::setDefaultStorageNumDrawCommands(int num)  { _defaultStorageNumDrawCommands=num; }
      inline bool AttribConfig::operator==(const AttribConfig &rhs) const  { return _configData==rhs._configData; }
      inline bool AttribConfig::operator!=(const AttribConfig &rhs) const  { return _configData!=rhs._configData; }
      inline bool AttribConfig::operator<=(const AttribConfig &rhs) const  { return _configData<=rhs._configData; }
      inline bool AttribConfig::operator>=(const AttribConfig &rhs) const  { return _configData>=rhs._configData; }
      inline bool AttribConfig::operator<(const AttribConfig &rhs) const  { return _configData<rhs._configData; }
      inline bool AttribConfig::operator>(const AttribConfig &rhs) const  { return _configData>rhs._configData; }
      inline AttribConfig* AttribConfig::Factory::create(const ConfigData &config,RenderingContext *rc,
         AttribConfigList::iterator selfIterator)  { return create(config.attribTypes,config.ebo,config.id,rc,selfIterator); }
      inline AttribConfig* AttribConfig::Factory::create(const std::vector<AttribType>& attribTypes,bool ebo,
         RenderingContext *rc,AttribConfigList::iterator selfIterator)
         { return create(attribTypes,ebo,AttribConfig::getId(attribTypes,ebo),rc,selfIterator); }
      inline std::shared_ptr<AttribConfig::Factory>& AttribConfig::factory()  { return _factory; }
      inline void AttribConfig::setFactory(std::shared_ptr<AttribConfig::Factory>& f)  { _factory=f; }

      inline AttribConfig& AttribConfigRef::get() const  { return *_pointer; }
      inline AttribConfig& AttribConfigRef::operator*() const  { return *_pointer; }
      inline AttribConfig* AttribConfigRef::operator->() const  { return _pointer; }
      inline AttribConfigRef::AttribConfigRef() : _pointer(NULL)  {}
      inline AttribConfigRef::AttribConfigRef(AttribConfig &ac) : _pointer(&ac)  { if(_pointer) _pointer->addReference(); }
      inline AttribConfigRef::AttribConfigRef(const AttribConfigRef &acr) : _pointer(acr._pointer)  { if(_pointer) _pointer->addReference(); }
      inline AttribConfigRef::AttribConfigRef(AttribConfigRef &&acr) : _pointer(acr._pointer)  { if(_pointer) acr._pointer=NULL; }
      inline AttribConfigRef::AttribConfigRef(const AttribConfig::ConfigData &config)
         : AttribConfigRef(config,RenderingContext::current().get())  {}
      inline AttribConfigRef::AttribConfigRef(const AttribConfig::ConfigData &config,RenderingContext *rc)
         : AttribConfigRef(rc->getAttribConfig(config))  {}
      inline AttribConfigRef::AttribConfigRef(const std::vector<AttribType>& attribTypes,bool ebo)
         : AttribConfigRef(attribTypes,ebo,AttribConfig::getId(attribTypes,ebo))  {}
      inline AttribConfigRef::AttribConfigRef(const std::vector<AttribType>& attribTypes,bool ebo,AttribConfigId id)
         : AttribConfigRef(attribTypes,ebo,id,RenderingContext::current().get())  {}
      inline AttribConfigRef::AttribConfigRef(const std::vector<AttribType>& attribTypes,bool ebo,RenderingContext *rc)
         : AttribConfigRef(attribTypes,ebo,AttribConfig::getId(attribTypes,ebo),rc)  {}
      inline AttribConfigRef::AttribConfigRef(const std::vector<AttribType>& attribTypes,bool ebo,
            AttribConfigId id,RenderingContext *rc) : AttribConfigRef(rc->getAttribConfig(attribTypes,ebo,id))  {}
      inline AttribConfigRef::~AttribConfigRef()  { if(_pointer) _pointer->removeReference(); }
      inline AttribConfigRef& AttribConfigRef::operator=(const AttribConfigRef &ac)  { if(_pointer) _pointer->removeReference(); _pointer=ac._pointer; if(_pointer) _pointer->addReference(); return *this; }
      inline AttribConfigRef& AttribConfigRef::operator=(AttribConfigRef &&ac)  { if(_pointer) _pointer->removeReference(); _pointer=ac._pointer; ac._pointer=NULL; return *this; }
      inline bool AttribConfigRef::operator==(const AttribConfigRef &rhs) const  { return _pointer==rhs._pointer; }
      inline bool AttribConfigRef::operator!=(const AttribConfigRef &rhs) const  { return _pointer!=rhs._pointer; }
      inline bool AttribConfigRef::operator<=(const AttribConfigRef &rhs) const  { return _pointer==rhs._pointer || *_pointer<*rhs._pointer; }
      inline bool AttribConfigRef::operator>=(const AttribConfigRef &rhs) const  { return _pointer==rhs._pointer || *_pointer>*rhs._pointer; }
      inline bool AttribConfigRef::operator<(const AttribConfigRef &rhs) const  { return _pointer!=rhs._pointer && *_pointer<*rhs._pointer; }
      inline bool AttribConfigRef::operator>(const AttribConfigRef &rhs) const  { return _pointer!=rhs._pointer && *_pointer>*rhs._pointer; }
   }
}

#endif // GE_RG_ATTRIB_CONFIG_H

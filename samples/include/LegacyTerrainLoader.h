/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
-----------------------------------------------------------------------------
*/

#include <OgreTerrain.h>
#include <OgreTerrainGroup.h>
#include <OgreTerrainMaterialGeneratorA.h>
#include <OgreConfigFile.h>

namespace Ogre
{
class CustomMatProfile : public Ogre::TerrainMaterialGenerator::Profile
{
    MaterialPtr mMaterial;
    bool mIsInit;
    bool mNormalMapRequired;
public:
    CustomMatProfile(const String& matName) : Profile(NULL, "", ""), mIsInit(false), mNormalMapRequired(true)
    {
        auto terrainGlobals = TerrainGlobalOptions::getSingletonPtr();
        mMaterial =
            MaterialManager::getSingleton().getByName(matName, terrainGlobals->getDefaultResourceGroup());
    }

    bool isVertexCompressionSupported() const { return false; }

    void setNormalMapRequired(bool enable) { mNormalMapRequired = enable; }

    MaterialPtr generate(const Terrain* terrain)
    {
        if (!mIsInit && mNormalMapRequired)
        {
            // Get default pass
            Pass *p = mMaterial->getTechnique(0)->getPass(0);	

            // Add terrain's global normalmap to renderpass so the fragment program can find it.
            p->createTextureUnitState()->_setTexturePtr(terrain->getTerrainNormalMap());

        }
        mIsInit = true;

        return mMaterial;
    }
    MaterialPtr generateForCompositeMap(const Terrain* terrain)
    {
        return terrain->_getCompositeMapMaterial();
    }
    void updateCompositeMap(const Terrain* terrain, const Rect& rect) {}
    void setLightmapEnabled(bool enabled) {}
    uint8 getMaxLayers(const Terrain* terrain) const { return 0; }

    void updateParams(const MaterialPtr& mat, const Terrain* terrain) {}
    void updateParamsForCompositeMap(const MaterialPtr& mat, const Terrain* terrain) {}
    void requestOptions(Terrain* terrain)
    {
        terrain->_setLightMapRequired(false);
        terrain->_setCompositeMapRequired(false);
        terrain->_setNormalMapRequired(mNormalMapRequired); 
    }
};
} // namespace Ogre

inline Ogre::TerrainGroup* loadLegacyTerrain(const Ogre::String& cfgFileName, Ogre::SceneManager* sceneMgr)
{
    using namespace Ogre;

    ConfigFile cfg;
    cfg.loadFromResourceSystem(cfgFileName, RGN_DEFAULT);
    cfg.getSettings();

    float worldSize;
    uint32 terrainSize;
    StringConverter::parse(cfg.getSetting("PageSize"), terrainSize);
    StringConverter::parse(cfg.getSetting("PageWorldX"), worldSize);

    auto terrainGlobals = TerrainGlobalOptions::getSingletonPtr();
    if(!terrainGlobals)
        terrainGlobals = new TerrainGlobalOptions();

    terrainGlobals->setMaxPixelError(StringConverter::parseReal(cfg.getSetting("MaxPixelError")));

    const String& customMatName = cfg.getSetting("CustomMaterialName");

    if(!customMatName.empty())
    {
        auto profile = new CustomMatProfile(customMatName);
        profile->setNormalMapRequired(StringConverter::parseBool(cfg.getSetting("VertexNormals")));
        terrainGlobals->getDefaultMaterialGenerator()->setActiveProfile(profile);
    }
    else 
    {
        auto profile = static_cast<TerrainMaterialGeneratorA::SM2Profile *>(
                terrainGlobals->getDefaultMaterialGenerator()->getActiveProfile());
        profile->setLayerSpecularMappingEnabled(false);
        profile->setLayerNormalMappingEnabled(false);
        profile->setLightmapEnabled(false); // baked into diffusemap
    }

    auto terrainGroup = new TerrainGroup(sceneMgr, Terrain::ALIGN_X_Z, terrainSize, worldSize);
    terrainGroup->setOrigin(Vector3(worldSize/2, 0, worldSize/2));

    auto &defaultimp = terrainGroup->getDefaultImportSettings();
    defaultimp.terrainSize = terrainSize;

    StringConverter::parse(cfg.getSetting("MaxHeight"), defaultimp.inputScale);
    defaultimp.maxBatchSize = StringConverter::parseInt(cfg.getSetting("TileSize"));
    defaultimp.minBatchSize = (defaultimp.maxBatchSize - 1)/2 + 1;

    const String& worldTexName = cfg.getSetting("WorldTexture");
    if(!worldTexName.empty())
    {
        defaultimp.layerList.resize(1);
        defaultimp.layerList[0].worldSize = worldSize; // covers whole terrain
        defaultimp.layerList[0].textureNames.push_back(worldTexName);
        // avoid empty texture name - will otherwise not be used
        defaultimp.layerList[0].textureNames.push_back(worldTexName);
    }

    // Load terrain from heightmap
    Image img;
    img.load(cfg.getSetting("Heightmap.image"), terrainGlobals->getDefaultResourceGroup());
    terrainGroup->defineTerrain(0, 0, &img);

    // sync load since we want everything in place when we start
    terrainGroup->loadTerrain(0, 0, true);

    terrainGroup->freeTemporaryResources();

    return terrainGroup;
}

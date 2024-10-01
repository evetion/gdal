/******************************************************************************
 * $Id$
 *
 * Project:  GDAL Rasterlite driver
 * Purpose:  Implement GDAL Rasterlite support using OGR SQLite driver
 * Author:   Even Rouault, <even dot rouault at spatialys.com>
 *
 **********************************************************************
 * Copyright (c) 2009-2013, Even Rouault <even dot rouault at spatialys.com>
 *
 * SPDX-License-Identifier: MIT
 ****************************************************************************/

#ifndef RASTERLITE_DATASET_INCLUDED
#define RASTERLITE_DATASET_INCLUDED

#include "gdal_pam.h"
#include "ogr_api.h"

char **RasterliteGetTileDriverOptions(CSLConstList papszOptions);

GDALDatasetH RasterliteOpenSQLiteDB(const char *pszFilename,
                                    GDALAccess eAccess);
CPLString RasterliteGetPixelSizeCond(double dfPixelXSize, double dfPixelYSize,
                                     const char *pszTablePrefixWithDot = "");
CPLString RasterliteGetSpatialFilterCond(double minx, double miny, double maxx,
                                         double maxy);

class RasterliteBand;

/************************************************************************/
/* ==================================================================== */
/*                              RasterliteDataset                       */
/* ==================================================================== */
/************************************************************************/

class RasterliteDataset final : public GDALPamDataset
{
    friend class RasterliteBand;

  public:
    RasterliteDataset();
    RasterliteDataset(RasterliteDataset *poMainDS, int nLevel);

    virtual ~RasterliteDataset();

    virtual char **GetMetadataDomainList() override;
    virtual char **GetMetadata(const char *pszDomain) override;
    virtual const char *GetMetadataItem(const char *pszName,
                                        const char *pszDomain) override;
    virtual CPLErr GetGeoTransform(double *padfGeoTransform) override;

    const OGRSpatialReference *GetSpatialRef() const override;

    virtual char **GetFileList() override;

    virtual CPLErr IBuildOverviews(const char *pszResampling, int nOverviews,
                                   const int *panOverviewList, int nBands,
                                   const int *panBandList,
                                   GDALProgressFunc pfnProgress,
                                   void *pProgressData,
                                   CSLConstList papszOptions) override;

    static GDALDataset *Open(GDALOpenInfo *);

  protected:
    virtual int CloseDependentDatasets() override;

  private:
    int bMustFree;
    RasterliteDataset *poMainDS;
    int nLevel;

    char **papszMetadata;
    char **papszImageStructure;
    char **papszSubDatasets;

    int nResolutions;
    double *padfXResolutions;
    double *padfYResolutions;
    RasterliteDataset **papoOverviews;
    int nLimitOvrCount;

    int bValidGeoTransform;
    double adfGeoTransform[6];
    OGRSpatialReference m_oSRS{};

    GDALColorTable *poCT;

    CPLString osTableName;
    CPLString osFileName;

    int bCheckForExistingOverview;
    CPLString osOvrFileName;

    GDALDatasetH hDS;

    int m_nLastBadTileId = -1;

    void AddSubDataset(const char *pszDSName);
    int GetBlockParams(OGRLayerH hRasterLyr, int nLevel, int *pnBands,
                       GDALDataType *peDataType, int *pnBlockXSize,
                       int *pnBlockYSize);
    CPLErr CleanOverviews();
    CPLErr CleanOverviewLevel(int nOvrFactor);
    CPLErr ReloadOverviews();
    CPLErr CreateOverviewLevel(const char *pszResampling, int nOvrFactor,
                               CSLConstList papszOptions,
                               GDALProgressFunc pfnProgress,
                               void *pProgressData);
};

/************************************************************************/
/* ==================================================================== */
/*                              RasterliteBand                          */
/* ==================================================================== */
/************************************************************************/

class RasterliteBand final : public GDALPamRasterBand
{
    friend class RasterliteDataset;

  public:
    RasterliteBand(RasterliteDataset *poDS, int nBand, GDALDataType eDataType,
                   int nBlockXSize, int nBlockYSize);

    virtual GDALColorInterp GetColorInterpretation() override;
    virtual GDALColorTable *GetColorTable() override;

    virtual int GetOverviewCount() override;
    virtual GDALRasterBand *GetOverview(int nLevel) override;

    virtual CPLErr IReadBlock(int, int, void *) override;
};

GDALDataset *RasterliteCreateCopy(const char *pszFilename, GDALDataset *poSrcDS,
                                  int bStrict, char **papszOptions,
                                  GDALProgressFunc pfnProgress,
                                  void *pProgressData);

CPLErr RasterliteDelete(const char *pszFilename);

#endif  // RASTERLITE_DATASET_INCLUDED

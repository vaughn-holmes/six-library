/* =========================================================================
 * This file is part of NITRO
 * =========================================================================
 *
 * (C) Copyright 2004 - 2014, MDA Information Systems LLC
 *
 * NITRO is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, If not,
 * see <http://www.gnu.org/licenses/>.
 *
 */


#include <import/nitf.h>

NITF_CXX_GUARD

static nitf_TREDescription description[] = {
    {NITF_BCS_N, 2, "Number of Related TREs", "RELATED_TRES" },
    {NITF_BCS_A, 120, "Related TREs", "ADDITIONAL_TRES" },
    {NITF_BCS_A, 12, "Processor Version No.", "RD_PRC_NO" },
    {NITF_BCS_A, 4, "VPH Processing Method", "IF_PROCESS" },
    {NITF_BCS_A, 4, "Nominal Center Freq Band", "RD_CEN_FREQ" },
    {NITF_BCS_A, 5, "Collection Mode", "RD_MODE" },
    {NITF_BCS_N, 4, "Data Patch Number Field", "RD_PATCH_NO" },
    {NITF_BCS_A, 5, "Complex Domain", "CMPLX_DOMAIN" },
    {NITF_BCS_A, 4, "Magnitude Remap", "CMPLX_MAG_REMAP_TYPE" },
    {NITF_BCS_N, 7, "Linear Scale Factor", "CMPLX_LIN_SCALE" },
    {NITF_BCS_N, 7, "Complex Average Power", "CMPLX_AVG_POWER" },
    {NITF_BCS_N, 5, "Complex LinLog Transition Point", "CMPLX_LINLOG_TP" },
    {NITF_BCS_A, 3, "Phase Quantization Flag", "CMPLX_PHASE_QUANT_FLAG" },
    {NITF_BCS_N, 2, "Phase Quantization Bit Depth",
     "CMPLX_PHASE_QUANT_BIT_DEPTH" },

    {NITF_BCS_N, 2, "Size of First Pix Component in Bits", "CMPLX_SIZE_1" },
    {NITF_BCS_N, 2, "Data Compression of First Pixel Component", "CMPLX_IC_1" },

    {NITF_BCS_N, 2, "Size of Second Pixel Component in Bits", "CMPLX_SIZE_2" },
    {NITF_BCS_N, 2, "Data Compression of Second Pixel Component", "CMPLX_IC_2" },

    {NITF_BCS_N, 5, "Complex Imagery Compressed Bits per Pixel", "CMPLX_IC_BPP" },
    {NITF_BCS_A, 3, "Type of Weighting", "CMPLX_WEIGHT" },
    {NITF_BCS_N, 2, "Azimuth Sidelobe Level", "CMPLX_AZ_SLL" },
    {NITF_BCS_N, 2, "Range Sidelobe Level", "CMPLX_RNG_SLL" },
    {NITF_BCS_N, 2, "Azimuth Taylor nbar", "CMPLX_AZ_TAY_NBAR" },
    {NITF_BCS_N, 2, "Range Taylor nbar", "CMPLX_RNG_TAY_NBAR" },
    {NITF_BCS_A, 3, "Weighting Normalization", "CMPLX_WEIGHT_NORM" },
    {NITF_BCS_A, 1, "Complex Signal PLane", "CMPLX_SIGNAL_PLANE" },
    {NITF_BCS_N, 6, "Sample Location of DC in row dimension", "IF_DC_SF_ROW" },
    {NITF_BCS_N, 6, "Sample Location of DC in column dimension", "IF_DC_SF_COL" },
    {NITF_BCS_N, 6, "UL Signal Corner, Row", "IF_PATCH_1_ROW" },
    {NITF_BCS_N, 6, "UL Signal Corner, Col", "IF_PATCH_1_COL" },
    {NITF_BCS_N, 6, "UR Signal Corner, Row", "IF_PATCH_2_ROW" },
    {NITF_BCS_N, 6, "UR Signal Corner, Col", "IF_PATCH_2_COL" },
    {NITF_BCS_N, 6, "BR Signal Corner, Row", "IF_PATCH_3_ROW" },
    {NITF_BCS_N, 6, "BR Signal Corner, Col", "IF_PATCH_3_COL" },
    {NITF_BCS_N, 6, "BL Signal Corner, Row", "IF_PATCH_4_ROW" },
    {NITF_BCS_N, 6, "BL Signal Corner, Col", "IF_PATCH_4_COL" },
    {NITF_BCS_N, 8, "IS DC Location Row", "IF_DC_IS_ROW" },
    {NITF_BCS_N, 8, "IS DC Location Column", "IF_DC_IS_COL" },
    {NITF_BCS_N, 8, "Row Location of Patch", "IF_IMG_ROW_DC" },
    {NITF_BCS_N, 8, "Column Location of Patch", "IF_IMG_COL_DC" },

    {NITF_BCS_N, 6, "UL Tile Corner, Row", "IF_TILE_1_ROW" },
    {NITF_BCS_N, 6, "UL Tile Corner, Column", "IF_TILE_1_COL" },
    {NITF_BCS_N, 6, "UR Tile Corner, Row", "IF_TILE_2_ROW" },
    {NITF_BCS_N, 6, "UR Tile Corner, Column", "IF_TILE_2_COL" },
    {NITF_BCS_N, 6, "LR Tile Corner, Row", "IF_TILE_3_ROW" },
    {NITF_BCS_N, 6, "LR Tile Corner, Column", "IF_TILE_3_COL" },
    {NITF_BCS_N, 6, "LL Tile Corner, Row", "IF_TILE_4_ROW" },
    {NITF_BCS_N, 6, "LL Tile Corner, Column", "IF_TILE_4_COL" },
    {NITF_BCS_A, 1, "Range Deskew Flag", "IF_RD" },
    {NITF_BCS_A, 1, "Range Walk Correction", "IF_RGWLK" },
    {NITF_BCS_A, 1, "Range Keystone/Curv. Cor", "IF_KEYSTN" },
    {NITF_BCS_A, 1, "Residual Linear Shift Co", "IF_LINSFT" },
    {NITF_BCS_A, 1, "Sub-patch Phase Correctn.", "IF_SUBPATCH" },
    {NITF_BCS_A, 1, "Other Geom. Dist. Crtns.", "IF_GEODIST" },
    {NITF_BCS_A, 1, "Range Fall-off Correction", "IF_RGFO" },
    {NITF_BCS_A, 1, "Antenna Beam Pattern Comp", "IF_BEAM_COMP" },
    {NITF_BCS_N, 8, "Range Direction Resolution", "IF_RGRES" },
    {NITF_BCS_N, 8, "Azimuth Resolution", "IF_AZRES" },
    {NITF_BCS_N, 8, "Range Sample Spacing", "IF_RSS" },
    {NITF_BCS_N, 8, "Azimuth Sample Spacing", "IF_AZSS" },
    {NITF_BCS_N, 8, "Range Sample Rate", "IF_RSR" },
    {NITF_BCS_N, 8, "Azimuth Sample Rate", "IF_AZSR" },
    {NITF_BCS_N, 7, "Original Range Samples", "IF_RFFT_SAMP" },
    {NITF_BCS_N, 7, "Original Azimuth Samples", "IF_AZFFT_SAMP" },
    {NITF_BCS_N, 7, "Total Range Samples", "IF_RFFT_TOT" },
    {NITF_BCS_N, 7, "Total Azimuth Samples", "IF_AZFFT_TOT" },
    {NITF_BCS_N, 6, "Sub-patch Size, Row", "IF_SUBP_ROW" },
    {NITF_BCS_N, 6, "Sub-patch Size, Column", "IF_SUBP_COL" },
    {NITF_BCS_N, 4, "Subpatch Counts, Range", "IF_SUB_RG" },
    {NITF_BCS_N, 4, "Subpatch Counts, Azimuth", "IF_SUB_AZ" },
    {NITF_BCS_A, 1, "FFT Sign Convention in Range", "IF_RFFTS" },
    {NITF_BCS_A, 1, "FFT Sign Convention in Azimuth", "IF_AFFTS" },
    {NITF_BCS_A, 7, "Range Data Range", "IF_RANGE_DATA" },
    {NITF_BCS_A, 1, "Increasing phase", "IF_INCPH" },
    {NITF_BCS_A, 8, "Super Res. Alg. Name (1st Iter)", "IF_SR_NAME1" },

    {NITF_BCS_N, 8, "Amount/Factor of Super Res. Applied (1st Iter)", "IF_SR_AMOUNT1" },
    {NITF_BCS_A, 8, "Super Res. Alg. Name (2nd Iter)", "IF_SR_NAME2" },
    {NITF_BCS_N, 8, "Amount/Factor of Super Res. Applied (2nd Iter)",
     "IF_SR_AMOUNT2" },
    {NITF_BCS_A, 8, "Super Res. Alg. Name (3rd Iter)", "IF_SR_NAME3" },
    {NITF_BCS_N, 8, "Amount/Factor of Super Res. Applied (3rd Iter)", "IF_SR_AMOUNT" },
    {NITF_BCS_A, 5, "First Autofocus Iteration", "AF_TYPE1" },
    {NITF_BCS_A, 5, "Second Autofocus Iteration", "AF_TYPE2" },
    {NITF_BCS_A, 5, "Third Autofocus Iteration", "AF_TYPE3" },
    {NITF_BCS_A, 1, "Transmit Polarization", "POL_TR" },
    {NITF_BCS_A, 1, "Receive Polarization", "POL_RE" },
    {NITF_BCS_A, 40, "Polarization Frame of Ref.", "POL_REFERENCE" },
    {NITF_BCS_A, 1, "Polarimetric Data Set", "POL" },
    {NITF_BCS_A, 1, "Pixel Registered", "POL_REG" },

    {NITF_BCS_N, 5, "Minimum Polarization Isolation", "POL_ISO_1" },
    {NITF_BCS_A, 1, "RCS Gray Level Balancing", "POL_BAL" },
    {NITF_BCS_N, 8, "Pixel Amplitude Balance Coeff", "POL_BAL_MAG" },
    {NITF_BCS_N, 8, "Pixel Phase Balance Coeff", "POL_BAL_PHS" },
    {NITF_BCS_A, 1, "Radar Hardware Phase Bal.", "POL_HCOMP" },
    {NITF_BCS_A, 10, "Basis Set", "POL_HCOMP_BASIS" },
    {NITF_BCS_N, 9, "First Radar HW Phase Bal.", "POL_HCOMP_COEF_1" },
    {NITF_BCS_N, 9, "Second Radar HW Phase Bal", "POL_HCOMP_COEF_2" },
    {NITF_BCS_N, 9, "Third Radar HW Phase Bal.", "POL_HCOMP_COEF_3" },
    {NITF_BCS_N, 1, "Radar Autofocus Phase Bal.", "POL_AFCOMP" },
    {NITF_BCS_A, 15, "Spare alpha field", "POL_SPARE_A" },
    {NITF_BCS_N, 9, "Spare numeric field", "POL_SPARE_N" },
    {NITF_BCS_A, 9, "Collection Date", "T_UTC_YYYYMMMDD" },
    {NITF_BCS_N, 6, "Collection Time", "T_HHMMSSUTC" },
    {NITF_BCS_A, 6, "Civil Time of Collection", "T_HHMMSSLOCAL" },
    {NITF_BCS_N, 11, "Slant Range at Sensor Reference Center", "CG_SRAC" },
    {NITF_BCS_N, 7, "Slant Range 95% Confidence Interval", "CG_SLANT_CONFIDENCE" },
    {NITF_BCS_N, 11, "Cross Track Range at Sensor Reference Center", "CG_CROSS" },
    {NITF_BCS_N, 7, "Cross Track Range 95% Confidence Interval", "CG_CROSS_CONFIDENCE" },
    {NITF_BCS_N, 9, "Cone Angle at Sensor Reference Point", "CG_CAAC" },
    {NITF_BCS_N, 6, "Cone Angle 95% Confidence", "CG_CONE_CONFIDENCE" },
    {NITF_BCS_N, 8, "Ground Plane Squint Angle", "CG_GPSAC" },
    {NITF_BCS_N, 6, "Squint Angle 95% Confidence", "CG_GPSAC_CONFIDENCE" },
    {NITF_BCS_N, 8, "Slant Plane Squint Angle", "CG_SQUINT" },
    {NITF_BCS_N, 7, "Grazing Angle at Sensor Reference Point Center", "CG_GAAC" },
    {NITF_BCS_N, 6, "Grazing Angle at Sensor Reference Point Center 95% Confidence", "CG_GAAC_CONFIDENCE" },
    {NITF_BCS_N, 7, "Incidence angle", "CG_INCIDENT" },
    {NITF_BCS_N, 7, "Slope angle", "CG_SLOPE" },
    {NITF_BCS_N, 8, "Tilt angle", "CG_TILT" },
    {NITF_BCS_A, 1, "Look Direction", "CG_LD" },
    {NITF_BCS_N, 8, "North Relative to Top Image Edge", "CG_NORTH" },
    {NITF_BCS_N, 6, "North Angle 95% Confidence", "CG_NORTH_CONFIDENCE" },
    {NITF_BCS_N, 8, "East Relative to Top Image Edge", "CG_EAST" },
    {NITF_BCS_N, 8, "Range LOS Relative to Top Image Edge", "CG_RLOS" },
    {NITF_BCS_N, 6, "Range LOS 95% Confidence", "CG_LOS_CONFIDENCE" },
    {NITF_BCS_N, 8, "Layover Angle", "CG_LAYOVER" },
    {NITF_BCS_N, 8, "Shadow Angle", "CG_SHADOW" },
    {NITF_BCS_N, 7, "Out of Plane Motion", "CG_OPM" },
    {NITF_BCS_A, 5, "Nominal Geometry Reference", "CG_MODEL" },
    {NITF_BCS_N, 13, "Aimpoint of Antenna, x", "CG_AMPT_X" },
    {NITF_BCS_N, 13, "Aimpoint of Antenna, y", "CG_AMPT_Y" },
    {NITF_BCS_N, 13, "Aimpoint of Antenna, z", "CG_AMPT_Z" },
    {NITF_BCS_N, 6, "Aimpoint X,Y 95% Confidence", "CG_AP_CONF_XY" },
    {NITF_BCS_N, 6, "Aimpoint Z 95% Confidence", "CG_AP_CONF_Z" },
    {NITF_BCS_N, 13, "Sensor Reference Point, x", "CG_APCEN_X" },
    {NITF_BCS_N, 13, "Sensor Reference Point, y", "CG_APCEN_Y" },
    {NITF_BCS_N, 13, "Sensor Reference Point, z", "CG_APCEN_Z" },
    {NITF_BCS_N, 6, "SRP X,Y 95% Confidence", "CG_APER_CONF_XY" },
    {NITF_BCS_N, 6, "SRP Z 95% Confidence", "CG_APER_CONF_Z" },
    {NITF_BCS_N, 9, "Focus Plane Normal UV, x", "CG_FPNUV_X" },
    {NITF_BCS_N, 9, "Focus Plane Normal UV, y", "CG_FPNUV_Y" },
    {NITF_BCS_N, 9, "Focus Plane Normal UV, z", "CG_FPNUV_Z" },
    {NITF_BCS_N, 9, "Image Display Plane NUV, x", "CG_IDPNUVX" },
    {NITF_BCS_N, 9, "Image Display Plane NUV, y", "CG_IDPNUVY" },
    {NITF_BCS_N, 9, "Image Display Plane NUV, z", "CG_IDPNUVZ" },
    {NITF_BCS_N, 13, "Scene Center, x", "CG_SCECN_X" },
    {NITF_BCS_N, 13, "Scene Center, y", "CG_SCECN_Y" },
    {NITF_BCS_N, 13, "Scene Center, z", "CG_SCECN_Z" },
    {NITF_BCS_N, 6, "Scene Center X,Y 95% Conf", "CG_SC_CONF_XY" },
    {NITF_BCS_N, 6, "Scene Center Z 95% Confid", "CG_SC_CONF_Z" },
    {NITF_BCS_N, 8, "Swath Width", "CG_SWWD" },
    {NITF_BCS_N, 10, "Sensor Nominal Velocity, x", "CG_SNVEL_X" },
    {NITF_BCS_N, 10, "Sensor Nominal Velocity, y", "CG_SNVEL_Y" },
    {NITF_BCS_N, 10, "Sensor Nominal Velocity, z", "CG_SNVEL_Z" },
    {NITF_BCS_N, 10, "Sensor Nominal Accel, x", "CG_SNACC_X" },
    {NITF_BCS_N, 10, "Sensor Nominal Accel, y", "CG_SNACC_Y" },
    {NITF_BCS_N, 10, "Sensor Nominal Accel, z", "CG_SNACC_Z" },
    {NITF_BCS_N, 8, "Sensor Nom. Attitude Roll", "CG_SNATT_ROLL" },
    {NITF_BCS_N, 8, "Sensor Nom. Attitude Pitch", "CG_SNATT_PITCH" },
    {NITF_BCS_N, 8, "Sensor Nom. Attitude Yaw", "CG_SNATT_YAW" },
    {NITF_BCS_N, 9, "Geoid Tangent Plane Normal, x", "CG_GTP_X" },
    {NITF_BCS_N, 9, "Geoid Tangent Plane Normal, y", "CG_GTP_Y" },
    {NITF_BCS_N, 9, "Geoid Tangent Plane Normal, z", "CG_GTP_Z" },
    {NITF_BCS_A, 4, "Mapping Coordinate", "CG_MAP_TYPE" },
    {NITF_IF, 0, "eq GEOD", "CG_MAP_TYPE"},
    {NITF_BCS_N, 11, "Lat of the Patch Center", "CG_PATCH_LATCEN" },
    {NITF_BCS_N, 12, "Long of the Patch Center", "CG_PATCH_LNGCEN" },
    {NITF_BCS_N, 11, "Lat of Patch Corner, upper left", "CG_PATCH_LTCORUL" },
    {NITF_BCS_N, 12, "Lon of Patch Corner, upper left", "CG_PATCH_LGCORUL" },
    {NITF_BCS_N, 11, "Lat of Patch Corner, upper right", "CG_PATCH_LTCORUR" },
    {NITF_BCS_N, 12, "Lon of Patch Corner, upper right", "CG_PATCH_LGCORUR" },
    {NITF_BCS_N, 11, "Lat of Patch Corner, lower right", "CG_PATCH_LTCORLR" },
    {NITF_BCS_N, 12, "Lon of Patch Corner, lower right", "CG_PATCH_LGCORLR" },
    {NITF_BCS_N, 11, "Lat of Patch Corner, lower left", "CG_PATCH_LTCORLL" },
    {NITF_BCS_N, 12, "Lon of Patch Corner, lower left", "CG_PATCH_LNGCOLL" },
    {NITF_BCS_N, 9, "Latitude 95% Confidence", "CG_PATCH_LAT_CONFIDENCE" },
    {NITF_BCS_N, 9, "Longitude 95% Confidence", "CG_PATCH_LONG_CONFIDENCE" },
    {NITF_ENDIF, 0, NULL, NULL},
    {NITF_IF, 0, "eq MGRS", "CG_MAP_TYPE"},
    {NITF_BCS_A, 23, "MGRS Image Center", "CG_MGRS_CENT" },
    {NITF_BCS_A, 23, "MGRS Image Upper Left Corn", "CG_MGRSCORUL" },
    {NITF_BCS_A, 23, "MGRS Image Upper RightCorn", "CG_MGRSCORUR" },
    {NITF_BCS_A, 23, "MGRS Image Lower RightCorn", "CG_MGRSCORLR" },
    {NITF_BCS_A, 23, "MGRS Image Lower Left Corn", "CG_MGRCORLL" },
    {NITF_BCS_N, 7, "MGRS 95% Confidence", "CG_MGRS_CONFIDENCE" },
    {NITF_BCS_A, 11, "MGRS Blank Padding", "CG_MGRS_PAD" },
    {NITF_ENDIF, 0, NULL, NULL},
    {NITF_IF, 0, "eq NA", "CG_MAP_TYPE"},
    {NITF_BCS_A, 133, "Map Coordinate Padding", "CG_MAP_TYPE_BLANK" },
    {NITF_ENDIF, 0, NULL, NULL},
    {NITF_BCS_A, 144, "Spare Alpha field", "CG_SPARE_A" },
    {NITF_BCS_N, 7, "Radiometric Calibration Parameter", "CA_CALPA" },
    {NITF_BCS_N, 14, "Chirp Start Frequency", "WF_SRTFR" },
    {NITF_BCS_N, 14, "Chirp End Frequency", "WF_ENDFR" },
    {NITF_BCS_N, 10, "Chirp Rate", "WF_CHRPRT" },
    {NITF_BCS_N, 9, "Pulsewidth", "WF_WIDTH" },
    {NITF_BCS_N, 13, "Center frequency", "WF_CENFRQ" },
    {NITF_BCS_N, 13, "Chirp Bandwidth", "WF_BW" },
    {NITF_BCS_N, 7, "Pulse Repetition Frequency", "WF_PRF" },
    {NITF_BCS_N, 9, "Pulse Repetition Interval", "WF_PRI" },
    {NITF_BCS_N, 7, "Coherent Data Period", "WF_CDP" },
    {NITF_BCS_N, 9, "Number of Pulses", "WF_NUMBER_OF_PULSES" },
    {NITF_BCS_A, 1, "VPH Data", "VPH_COND" },
    {NITF_END, 0, NULL, NULL}
};

NITF_DECLARE_SINGLE_PLUGIN(CMETAA, description)

NITF_CXX_ENDGUARD


# -*- coding: utf-8 -*-

__all__ = [ 'RM_Peripheral_AGC_NS' ]

from . static import Base_RM_Peripheral
from . AGC_NS_register import *

class RM_Peripheral_AGC_NS(Base_RM_Peripheral):
    def __init__(self, rmio, label):
        self.__dict__['zz_frozen'] = False
        super(RM_Peripheral_AGC_NS, self).__init__(rmio, label,
            0xB800C000, 'AGC_NS',
            u"",
            [])
        self.IPVERSION = RM_Register_AGC_NS_IPVERSION(self.zz_rmio, self.zz_label)
        self.zz_rdict['IPVERSION'] = self.IPVERSION
        self.EN = RM_Register_AGC_NS_EN(self.zz_rmio, self.zz_label)
        self.zz_rdict['EN'] = self.EN
        self.STATUS0 = RM_Register_AGC_NS_STATUS0(self.zz_rmio, self.zz_label)
        self.zz_rdict['STATUS0'] = self.STATUS0
        self.STATUS1 = RM_Register_AGC_NS_STATUS1(self.zz_rmio, self.zz_label)
        self.zz_rdict['STATUS1'] = self.STATUS1
        self.STATUS2 = RM_Register_AGC_NS_STATUS2(self.zz_rmio, self.zz_label)
        self.zz_rdict['STATUS2'] = self.STATUS2
        self.RSSI = RM_Register_AGC_NS_RSSI(self.zz_rmio, self.zz_label)
        self.zz_rdict['RSSI'] = self.RSSI
        self.FRAMERSSI = RM_Register_AGC_NS_FRAMERSSI(self.zz_rmio, self.zz_label)
        self.zz_rdict['FRAMERSSI'] = self.FRAMERSSI
        self.CTRL0 = RM_Register_AGC_NS_CTRL0(self.zz_rmio, self.zz_label)
        self.zz_rdict['CTRL0'] = self.CTRL0
        self.CTRL1 = RM_Register_AGC_NS_CTRL1(self.zz_rmio, self.zz_label)
        self.zz_rdict['CTRL1'] = self.CTRL1
        self.CTRL2 = RM_Register_AGC_NS_CTRL2(self.zz_rmio, self.zz_label)
        self.zz_rdict['CTRL2'] = self.CTRL2
        self.CTRL3 = RM_Register_AGC_NS_CTRL3(self.zz_rmio, self.zz_label)
        self.zz_rdict['CTRL3'] = self.CTRL3
        self.CTRL4 = RM_Register_AGC_NS_CTRL4(self.zz_rmio, self.zz_label)
        self.zz_rdict['CTRL4'] = self.CTRL4
        self.CTRL5 = RM_Register_AGC_NS_CTRL5(self.zz_rmio, self.zz_label)
        self.zz_rdict['CTRL5'] = self.CTRL5
        self.CTRL6 = RM_Register_AGC_NS_CTRL6(self.zz_rmio, self.zz_label)
        self.zz_rdict['CTRL6'] = self.CTRL6
        self.CTRL7 = RM_Register_AGC_NS_CTRL7(self.zz_rmio, self.zz_label)
        self.zz_rdict['CTRL7'] = self.CTRL7
        self.RSSISTEPTHR = RM_Register_AGC_NS_RSSISTEPTHR(self.zz_rmio, self.zz_label)
        self.zz_rdict['RSSISTEPTHR'] = self.RSSISTEPTHR
        self.MANGAIN = RM_Register_AGC_NS_MANGAIN(self.zz_rmio, self.zz_label)
        self.zz_rdict['MANGAIN'] = self.MANGAIN
        self.IF = RM_Register_AGC_NS_IF(self.zz_rmio, self.zz_label)
        self.zz_rdict['IF'] = self.IF
        self.IEN = RM_Register_AGC_NS_IEN(self.zz_rmio, self.zz_label)
        self.zz_rdict['IEN'] = self.IEN
        self.CMD = RM_Register_AGC_NS_CMD(self.zz_rmio, self.zz_label)
        self.zz_rdict['CMD'] = self.CMD
        self.GAINRANGE = RM_Register_AGC_NS_GAINRANGE(self.zz_rmio, self.zz_label)
        self.zz_rdict['GAINRANGE'] = self.GAINRANGE
        self.AGCPERIOD0 = RM_Register_AGC_NS_AGCPERIOD0(self.zz_rmio, self.zz_label)
        self.zz_rdict['AGCPERIOD0'] = self.AGCPERIOD0
        self.AGCPERIOD1 = RM_Register_AGC_NS_AGCPERIOD1(self.zz_rmio, self.zz_label)
        self.zz_rdict['AGCPERIOD1'] = self.AGCPERIOD1
        self.HICNTREGION0 = RM_Register_AGC_NS_HICNTREGION0(self.zz_rmio, self.zz_label)
        self.zz_rdict['HICNTREGION0'] = self.HICNTREGION0
        self.HICNTREGION1 = RM_Register_AGC_NS_HICNTREGION1(self.zz_rmio, self.zz_label)
        self.zz_rdict['HICNTREGION1'] = self.HICNTREGION1
        self.STEPDWN = RM_Register_AGC_NS_STEPDWN(self.zz_rmio, self.zz_label)
        self.zz_rdict['STEPDWN'] = self.STEPDWN
        self.GAINSTEPLIM0 = RM_Register_AGC_NS_GAINSTEPLIM0(self.zz_rmio, self.zz_label)
        self.zz_rdict['GAINSTEPLIM0'] = self.GAINSTEPLIM0
        self.GAINSTEPLIM1 = RM_Register_AGC_NS_GAINSTEPLIM1(self.zz_rmio, self.zz_label)
        self.zz_rdict['GAINSTEPLIM1'] = self.GAINSTEPLIM1
        self.PNRFATT0 = RM_Register_AGC_NS_PNRFATT0(self.zz_rmio, self.zz_label)
        self.zz_rdict['PNRFATT0'] = self.PNRFATT0
        self.PNRFATT1 = RM_Register_AGC_NS_PNRFATT1(self.zz_rmio, self.zz_label)
        self.zz_rdict['PNRFATT1'] = self.PNRFATT1
        self.PNRFATT2 = RM_Register_AGC_NS_PNRFATT2(self.zz_rmio, self.zz_label)
        self.zz_rdict['PNRFATT2'] = self.PNRFATT2
        self.PNRFATT3 = RM_Register_AGC_NS_PNRFATT3(self.zz_rmio, self.zz_label)
        self.zz_rdict['PNRFATT3'] = self.PNRFATT3
        self.PNRFATT4 = RM_Register_AGC_NS_PNRFATT4(self.zz_rmio, self.zz_label)
        self.zz_rdict['PNRFATT4'] = self.PNRFATT4
        self.PNRFATT5 = RM_Register_AGC_NS_PNRFATT5(self.zz_rmio, self.zz_label)
        self.zz_rdict['PNRFATT5'] = self.PNRFATT5
        self.PNRFATT6 = RM_Register_AGC_NS_PNRFATT6(self.zz_rmio, self.zz_label)
        self.zz_rdict['PNRFATT6'] = self.PNRFATT6
        self.PNRFATT7 = RM_Register_AGC_NS_PNRFATT7(self.zz_rmio, self.zz_label)
        self.zz_rdict['PNRFATT7'] = self.PNRFATT7
        self.PNRFATT8 = RM_Register_AGC_NS_PNRFATT8(self.zz_rmio, self.zz_label)
        self.zz_rdict['PNRFATT8'] = self.PNRFATT8
        self.PNRFATT9 = RM_Register_AGC_NS_PNRFATT9(self.zz_rmio, self.zz_label)
        self.zz_rdict['PNRFATT9'] = self.PNRFATT9
        self.PNRFATT10 = RM_Register_AGC_NS_PNRFATT10(self.zz_rmio, self.zz_label)
        self.zz_rdict['PNRFATT10'] = self.PNRFATT10
        self.PNRFATT11 = RM_Register_AGC_NS_PNRFATT11(self.zz_rmio, self.zz_label)
        self.zz_rdict['PNRFATT11'] = self.PNRFATT11
        self.PNRFATTALT = RM_Register_AGC_NS_PNRFATTALT(self.zz_rmio, self.zz_label)
        self.zz_rdict['PNRFATTALT'] = self.PNRFATTALT
        self.LNAMIXCODE0 = RM_Register_AGC_NS_LNAMIXCODE0(self.zz_rmio, self.zz_label)
        self.zz_rdict['LNAMIXCODE0'] = self.LNAMIXCODE0
        self.LNAMIXCODE1 = RM_Register_AGC_NS_LNAMIXCODE1(self.zz_rmio, self.zz_label)
        self.zz_rdict['LNAMIXCODE1'] = self.LNAMIXCODE1
        self.PGACODE0 = RM_Register_AGC_NS_PGACODE0(self.zz_rmio, self.zz_label)
        self.zz_rdict['PGACODE0'] = self.PGACODE0
        self.PGACODE1 = RM_Register_AGC_NS_PGACODE1(self.zz_rmio, self.zz_label)
        self.zz_rdict['PGACODE1'] = self.PGACODE1
        self.LBT = RM_Register_AGC_NS_LBT(self.zz_rmio, self.zz_label)
        self.zz_rdict['LBT'] = self.LBT
        self.MIRRORIF = RM_Register_AGC_NS_MIRRORIF(self.zz_rmio, self.zz_label)
        self.zz_rdict['MIRRORIF'] = self.MIRRORIF
        self.SEQIF = RM_Register_AGC_NS_SEQIF(self.zz_rmio, self.zz_label)
        self.zz_rdict['SEQIF'] = self.SEQIF
        self.SEQIEN = RM_Register_AGC_NS_SEQIEN(self.zz_rmio, self.zz_label)
        self.zz_rdict['SEQIEN'] = self.SEQIEN
        self.RSSIABSTHR = RM_Register_AGC_NS_RSSIABSTHR(self.zz_rmio, self.zz_label)
        self.zz_rdict['RSSIABSTHR'] = self.RSSIABSTHR
        self.LNABOOST = RM_Register_AGC_NS_LNABOOST(self.zz_rmio, self.zz_label)
        self.zz_rdict['LNABOOST'] = self.LNABOOST
        self.ANTDIV = RM_Register_AGC_NS_ANTDIV(self.zz_rmio, self.zz_label)
        self.zz_rdict['ANTDIV'] = self.ANTDIV
        self.DUALRFPKDTHD0 = RM_Register_AGC_NS_DUALRFPKDTHD0(self.zz_rmio, self.zz_label)
        self.zz_rdict['DUALRFPKDTHD0'] = self.DUALRFPKDTHD0
        self.DUALRFPKDTHD1 = RM_Register_AGC_NS_DUALRFPKDTHD1(self.zz_rmio, self.zz_label)
        self.zz_rdict['DUALRFPKDTHD1'] = self.DUALRFPKDTHD1
        self.SPARE = RM_Register_AGC_NS_SPARE(self.zz_rmio, self.zz_label)
        self.zz_rdict['SPARE'] = self.SPARE
        self.__dict__['zz_frozen'] = True
#ifndef __COMMON_PART_HPP__
#define __COMMON_PART_HPP__

#include "ParticleCommon.hpp"

namespace Retro
{
namespace DNAParticle
{

template <class IDType>
struct GPSM : BigYAML
{
    static const char* DNAType() {return "Retro::GPSM";}
    const char* DNATypeV() const {return DNAType();}


    VectorElementFactory x18_POFS;
    IntElementFactory x1c_PMED;
    RealElementFactory x20_LENG;
    RealElementFactory x24_WIDT;
    IntElementFactory x28_MAXP;
    RealElementFactory x2c_GRTE;
    ColorElementFactory x30_COLR;
    IntElementFactory x34_LTME;
    VectorElementFactory x38_ILOC;
    VectorElementFactory x3c_IVEC;
    //EmitterElementFactory x40_EMTR;
    bool x44_0_SORT : 1; bool x44_1_MBLR : 1;  bool x44_2_LINE : 1; bool x44_3_LIT_ : 1;
    bool x44_4_AAPH : 1; bool x44_5_ZBUF : 1;  bool x44_6_FXLL : 1; bool x44_7_PMAB : 1;
    bool x45_0_VMD4 : 1; bool x45_1_VMD3 : 1;  bool x45_2_VMD2 : 1; bool x45_3_VMD1 : 1;
    bool x45_4_OPTS : 1; bool x45_5_PMUS : 1;  bool x45_6_PMOO : 1; // is the last bit unused?
    IntElementFactory x48_MBSP;
    RealElementFactory x4c_SIZE;
    RealElementFactory x50_ROTA;
    //TextureElementFactory x54_TEXR;
    //TextureElementFactory x58_TIND;
    // x5c_PMDL Model
    VectorElementFactory x6c_PMOP;
    VectorElementFactory x70_PMRT;
    VectorElementFactory x74_PMSC;
    ColorElementFactory x78_PMCL;
    ChildGeneratorDesc<IDType> x8c_ICTS;
    IntElementFactory x9c_NCSY;
    IntElementFactory xb4_NDSY;
    IntElementFactory xa0_CSSD;
    ChildGeneratorDesc<IDType> xa4_IDTS;
    IntElementFactory xc8_PISY;
    IntElementFactory xcc_SISY;
    IntElementFactory xe4_SSSD;
    VectorElementFactory xe8_SSPO;
    // xec_PMLC Electric Generator
    IntElementFactory x100_LTYP;
    ColorElementFactory x104_LCLR;
    RealElementFactory x108_LINT;
    VectorElementFactory x110_LDIR;
    VectorElementFactory x10c_LOFF;
    IntElementFactory x114_LFOT;
    RealElementFactory x118_LFOR;
    RealElementFactory x11c_LSLA;

    void read(Athena::io::YAMLDocReader& r)
    {
        if (r.enterSubRecord("PMCL"))
        {
            x78_PMCL.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("LFOR"))
        {
            x118_LFOR.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("IDTS"))
        {
            xa4_IDTS.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("EMTR"))
        {
            //x40_EMTR.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("COLR"))
        {
            x30_COLR.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("CIND"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("AAPH"))
        {
            x44_4_AAPH = r.readBool(nullptr);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("CSSD"))
        {
            xa0_CSSD.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("GRTE"))
        {
            x2c_GRTE.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("FXLL"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("ICTS"))
        {
            x8c_ICTS.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("KSSM"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("ILOC"))
        {
            x38_ILOC.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("IITS"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("IVEC"))
        {
            x3c_IVEC.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("LDIR"))
        {
            x110_LDIR.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("COLR"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("LCLR"))
        {
            x104_LCLR.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("LENG"))
        {
            x20_LENG.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("MAXP"))
        {
            x28_MAXP.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("LOFF"))
        {
            x10c_LOFF.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("LINT"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("LINE"))
        {
            x44_2_LINE = r.readBool(nullptr);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("LFOT"))
        {
            x114_LFOT.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("LIT_"))
        {
            x44_3_LIT_ = r.readBool(nullptr);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("LTME"))
        {
            x34_LTME.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("LSLA"))
        {
            x11c_LSLA.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("LTYP"))
        {
            x100_LTYP.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("NDSY"))
        {
            xb4_NDSY.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("MBSP"))
        {
            x48_MBSP.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("MBLR"))
        {
            x44_1_MBLR = r.readBool(nullptr);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("NCSY"))
        {
            x9c_NCSY.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("PISY"))
        {
            xc8_PISY.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("OPTS"))
        {
            x45_4_OPTS = r.readBool(nullptr);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("PMAB"))
        {
            x44_7_PMAB = r.readBool(nullptr);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("SESD"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("PSLT"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("PMSC"))
        {
            x74_PMSC.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("PMOP"))
        {
            x6c_PMOP.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("PMDL"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("PMOO"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("PMRT"))
        {
            x70_PMRT.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("POFS"))
        {
            x18_POFS.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("PMUS"))
        {
            x45_5_PMUS = r.readBool(nullptr);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("PSIV"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("ROTA"))
        {
            x50_ROTA.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("PSVM"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("PSTS"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("PSOV"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("PSWT"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("PMLC"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("PMED"))
        {
            x1c_PMED.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("PMOO"))
        {
            x45_6_PMOO = r.readBool(nullptr);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("SSSD"))
        {
            xe4_SSSD.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("SORT"))
        {
            x44_0_SORT = r.readBool(nullptr);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("SIZE"))
        {
            x4c_SIZE.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("SISY"))
        {
            xcc_SISY.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("SSPO"))
        {
            xe8_SSPO.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("TEXR"))
        {
            //x54_TEXR.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("SSWH"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("TIND"))
        {
            //x58_TIND.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("VMD4"))
        {
            x45_0_VMD4 = r.readBool(nullptr);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("VMD3"))
        {
            x45_1_VMD3 = r.readBool(nullptr);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("VMD2"))
        {
            x45_2_VMD2 = r.readBool(nullptr);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("VMD1"))
        {
            x45_3_VMD1 = r.readBool(nullptr);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("VEL4"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("VEL3"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("VEL2"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("VEL1"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("ZBUF"))
        {
            x44_5_ZBUF = r.readBool(nullptr);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("WIDT"))
        {
            x24_WIDT.read(r);
            r.leaveSubRecord();
        }
    }
    void write(Athena::io::YAMLDocWriter& w) const
    {

    }
    size_t binarySize(size_t __isz) const
    {

    }
    void read(Athena::io::IStreamReader& r)
    {

    }
    void write(Athena::io::IStreamWriter& w) const
    {

    }
};

}
}

#endif // __COMMON_PART_HPP__
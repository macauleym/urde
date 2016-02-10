#ifndef __RETRO_CEMITTERELEMENT_HPP__
#define __RETRO_CEMITTERELEMENT_HPP__

#include "IElement.hpp"

namespace Retro
{

class CEESimpleEmitter : public CEmitterElement
{
    std::unique_ptr<CVectorElement> x4_loc;
    std::unique_ptr<CVectorElement> x8_vec;
public:
    CEESimpleEmitter(CVectorElement* a, CVectorElement* b)
    : x4_loc(a), x8_vec(b) {}
};

class CVESphere : public CEmitterElement
{
    std::unique_ptr<CVectorElement> x4_a;
    std::unique_ptr<CRealElement> x8_b;
    std::unique_ptr<CRealElement> xc_c;
public:
    CVESphere(CVectorElement* a, CRealElement* b, CRealElement* c)
    : x4_a(a), x8_b(b), xc_c(c) {}
};

class CVEAngleSphere : public CEmitterElement
{
    std::unique_ptr<CVectorElement> x4_a;
    std::unique_ptr<CRealElement> x8_b;
    std::unique_ptr<CRealElement> xc_c;
    std::unique_ptr<CRealElement> x10_d;
    std::unique_ptr<CRealElement> x14_e;
    std::unique_ptr<CRealElement> x18_f;
    std::unique_ptr<CRealElement> x1c_g;
public:
    CVEAngleSphere(CVectorElement* a, CRealElement* b, CRealElement* c, CRealElement* d,
                   CRealElement* e, CRealElement* f, CRealElement* g)
    : x4_a(a), x8_b(b), xc_c(c), x10_d(d), x14_e(e), x18_f(f), x1c_g(g) {}
};

}

#endif // __RETRO_CEMITTERELEMENT_HPP__
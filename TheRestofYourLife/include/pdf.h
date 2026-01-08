#ifndef PDF_H
#define PDF_H

#include "rtweekend.h"
#include "onb.h"
#include "hittable.h"

class Pdf {
public:
    virtual ~Pdf() {}

    virtual double value(const Vec3& direction) const = 0;
    virtual Vec3 generate() const = 0;
};

class CosinePdf : public Pdf {
public:
    CosinePdf(const Vec3& w) { uvw.build_from_w(w); }

    double value(const Vec3& direction) const override {
        auto cosine_theta = dot(unit_vector(direction), uvw.w());
        return std::fmax(0, cosine_theta / pi);
    }

    Vec3 generate() const override {
        return uvw.local(random_cosine_direction());
    }

private:
    Onb uvw;
};

class HittablePdf : public Pdf {
public:
    HittablePdf(const Hittable& _objects, const Point3& _origin)
        : objects(_objects), origin(_origin) {}

    double value(const Vec3& direction) const override {
        return objects.pdf_value(origin, direction);
    }
    Vec3 generate() const override {
        return objects.random(origin);
    }

private:
    const Hittable& objects;
    Point3 origin;
};

class MixturePdf : public Pdf {
public:
    MixturePdf(std::shared_ptr<Pdf> p0, std::shared_ptr<Pdf> p1) {
        p[0] = p0;
        p[1] = p1;
    }
    double value(const Vec3& direction) const override {
        return 0.5 * p[0]->value(direction) + 0.5 * p[1]->value(direction);
    }
    Vec3 generate() const override {
        if (random_double() < 0.5)
            return p[0]->generate();
        else
            return p[1]->generate();
    }
private:
    std::shared_ptr<Pdf> p[2];
};

#endif
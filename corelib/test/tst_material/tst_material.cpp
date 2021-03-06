#include "src/material.h"
#include <QtTest>
#include <cmath>

class tst_material : public QObject {
    Q_OBJECT

  public:
    tst_material();
    ~tst_material();

  private slots:
    void cook_torrance_name();
    void cook_torrance_sample_dir();
    void cook_torrance_brdf();
    void cook_torrance_special_case();

    void oren_nayar_name();
    void oren_nayar_sample_dir();
    void oren_nayar_brdf();
    void oren_nayar_special_case();
};

tst_material::tst_material() {}

tst_material::~tst_material() {}

void tst_material::cook_torrance_name() {
    e8::cook_torr mat =
        e8::cook_torr("test_cook_torr", e8util::vec3({0.787f, 0.787f, 0.787f}), 0.25f, 2.93f);
    QVERIFY(mat.name() == "test_cook_torr");
}

void tst_material::oren_nayar_name() {
    e8::oren_nayar mat =
        e8::oren_nayar("test_oren_nayar", e8util::vec3({0.725f, 0.710f, 0.680f}), 0.078f);
    QVERIFY(mat.name() == "test_oren_nayar");
}

void generic_validate_mat_dir(float *p_valid, std::vector<e8util::vec3> *samples,
                              std::vector<float> *solid_angle_dens, e8::if_material const &mat) {
    samples->clear();

    e8util::vec3 normal{0.0f, 0.0f, 1.0f};
    e8util::vec3 o_ray{1.0f, 1.0f, 1.0f};
    o_ray = o_ray.normalize();
    e8util::rng rng(13);

    unsigned const NUM_SAMPLES = 1000;
    for (unsigned i = 0; i < NUM_SAMPLES; i++) {
        float dens = -1;
        e8util::vec3 i_ray = mat.sample(&rng, &dens, /*uv=*/e8util::vec2(), normal, o_ray);

        QVERIFY(dens >= 0.0f);
        if (!e8util::equals(i_ray, e8util::vec3())) {
            QVERIFY(e8util::equals(i_ray.norm(), 1.0f));
            QVERIFY(i_ray.inner(normal) >= 0.0f);
            if (samples != nullptr) {
                samples->push_back(i_ray);
            }
            if (solid_angle_dens != nullptr) {
                solid_angle_dens->push_back(dens);
            }
        }
    }

    *p_valid = static_cast<float>(samples->size()) / NUM_SAMPLES;
}

void generic_validate_mat_brdf(e8::if_material const &mat) {
    float p_valid;
    std::vector<e8util::vec3> samples;
    std::vector<float> dens;
    generic_validate_mat_dir(&p_valid, &samples, &dens, mat);

    e8util::vec3 cumulative_density;
    for (unsigned i = 0; i < samples.size(); i++) {
        e8util::vec3 normal{0.0f, 0.0f, 1.0f};
        e8util::vec3 o_ray{1.0f, 1.0f, 1.0f};
        e8util::vec3 i_ray = samples[i];
        e8util::vec3 weight = mat.eval(/*uv=*/e8util::vec2(), normal, o_ray, i_ray);
        cumulative_density += weight * normal.inner(i_ray) / dens[i];

        QVERIFY(!std::isnan(weight(0)));
        QVERIFY(!std::isnan(weight(1)));
        QVERIFY(!std::isnan(weight(2)));

        QVERIFY(!std::isinf(weight(0)));
        QVERIFY(!std::isinf(weight(1)));
        QVERIFY(!std::isinf(weight(2)));

        QVERIFY(weight(0) >= 0.0f);
        QVERIFY(weight(1) >= 0.0f);
        QVERIFY(weight(2) >= 0.0f);

        e8util::vec3 reversed_weight = mat.eval(/*uv=*/e8util::vec2(), normal, i_ray, o_ray);

        QVERIFY(e8util::equals(weight(0), reversed_weight(0)));
        QVERIFY(e8util::equals(weight(1), reversed_weight(1)));
        QVERIFY(e8util::equals(weight(2), reversed_weight(2)));
    }
    cumulative_density *= 1.0f / static_cast<float>(samples.size());

    QVERIFY(cumulative_density(0) < 1.0f);
    QVERIFY(cumulative_density(1) < 1.0f);
    QVERIFY(cumulative_density(2) < 1.0f);
}

void tst_material::cook_torrance_sample_dir() {
    e8::cook_torr mat =
        e8::cook_torr("test_cook_torr", e8util::vec3({0.787f, 0.787f, 0.787f}), 0.2f, 2.93f);
    float p_valid;
    std::vector<e8util::vec3> samples;
    generic_validate_mat_dir(&p_valid, &samples, nullptr, mat);
    QVERIFY(p_valid >= .8f);
}

void tst_material::oren_nayar_sample_dir() {
    e8::oren_nayar mat =
        e8::oren_nayar("test_oren_nayar", e8util::vec3({0.725f, 0.710f, 0.680f}), 0.078f);
    float p_valid;
    std::vector<e8util::vec3> samples;
    generic_validate_mat_dir(&p_valid, &samples, nullptr, mat);
    QVERIFY(p_valid == 1.0f);
}

void tst_material::cook_torrance_brdf() {
    e8::cook_torr mat =
        e8::cook_torr("test_cook_torr", e8util::vec3({0.787f, 0.787f, 0.787f}), 0.25f, 2.93f);
    generic_validate_mat_brdf(mat);
}

void tst_material::oren_nayar_brdf() {
    e8::oren_nayar mat =
        e8::oren_nayar("test_oren_nayar", e8util::vec3({0.725f, 0.710f, 0.680f}), 0.078f);
    generic_validate_mat_brdf(mat);
}

void tst_material::cook_torrance_special_case() {
    e8::cook_torr mat =
        e8::cook_torr("test_cook_torr", e8util::vec3({0.787f, 0.787f, 0.787f}), 0.25f, 2.93f);
    e8util::vec3 i{0.983719f, 0.0456052f, 0.173832f};
    e8util::vec3 o{0.687788f, 0.361631f, 0.629421f};
    e8util::vec3 n{0.687654f, 0.361558f, 0.62961f};
    e8util::vec3 density = mat.eval(e8util::vec2(), n, o, i);

    QVERIFY(!std::isnan(density(0)));
    QVERIFY(!std::isnan(density(1)));
    QVERIFY(!std::isnan(density(2)));

    QVERIFY(!std::isinf(density(0)));
    QVERIFY(!std::isinf(density(1)));
    QVERIFY(!std::isinf(density(2)));

    QVERIFY(density(0) >= 0.0f);
    QVERIFY(density(1) >= 0.0f);
    QVERIFY(density(2) >= 0.0f);
}

void tst_material::oren_nayar_special_case() {
    e8::oren_nayar mat =
        e8::oren_nayar("test_oren_nayar", e8util::vec3({0.725f, 0.710f, 0.680f}), 0.078f);
    e8util::vec3 i{0.983719f, 0.0456052f, 0.173832f};
    e8util::vec3 o{0.687788f, 0.361631f, 0.629421f};
    e8util::vec3 n{0.687654f, 0.361558f, 0.62961f};
    e8util::vec3 density = mat.eval(e8util::vec2(), n, o, i);

    QVERIFY(!std::isnan(density(0)));
    QVERIFY(!std::isnan(density(1)));
    QVERIFY(!std::isnan(density(2)));

    QVERIFY(!std::isinf(density(0)));
    QVERIFY(!std::isinf(density(1)));
    QVERIFY(!std::isinf(density(2)));

    QVERIFY(density(0) >= 0.0f);
    QVERIFY(density(1) >= 0.0f);
    QVERIFY(density(2) >= 0.0f);
}

QTEST_APPLESS_MAIN(tst_material)

#include "tst_material.moc"

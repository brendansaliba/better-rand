#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <unistd.h>
#include <sys/random.h>

// based on pcg random number generator (https://www.pcg-random.org)
// inspired by @Magicalbat

#define PI 3.14159265358979

typedef struct {
    uint64_t state;
    uint64_t inc;
    float prev_norm;
} prng_state; 

void prng_seed_r(prng_state* rng, uint64_t initstate, uint64_t initseq);
void prng_seed(uint64_t initstate, uint64_t initseq);

uint32_t prng_rand_r(prng_state* rng);
uint32_t prng_rand(void);

float prng_randf_r(prng_state* rng);
float prng_randf(void);

float prng_randf_norm_r(prng_state* rng);
float prng_randf_norm(void);

void plat_get_entropy(void* data, uint32_t size);

int main(void) {
    uint64_t seeds[2] = { 0 };
    plat_get_entropy(seeds, sizeof(seeds));

    prng_state rng = { };
    prng_seed_r(&rng, seeds[0], seeds[1]);

    for (uint32_t i = 0; i< 10; i++ ) {

        printf("%f\n", prng_randf_norm_r(&rng)); 

    }

    return 0;
    
}

static prng_state s_prng_state = {
    0x853c49e6748fea9bULL, 0xda3e39cb94b95bdbULL,
    NAN
}; // pfm

void prng_seed_r(prng_state* rng, uint64_t initstate, uint64_t initseq) {
    rng->state = 0U;
    rng->inc = (initseq << 1u) | 1u;
    prng_rand_r(rng);
    rng->state += initstate;
    prng_rand_r(rng);   

    rng->prev_norm = NAN;
}

void prng_seed(uint64_t initstate, uint64_t initseq) {
    prng_seed_r(&s_prng_state, initstate, initseq);
}

uint32_t prng_rand_r(prng_state* rng) {
    uint64_t oldstate = rng->state;
    rng->state = oldstate * 6364136223846793005ULL + rng->inc;
    uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    uint32_t rot = oldstate >> 59u;
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31)); 
} // more pfm

uint32_t prng_rand(void) {
    return prng_rand_r(&s_prng_state);
}

float prng_randf_r(prng_state* rng) {
    return (float)prng_rand_r(rng) / (float)UINT32_MAX;
}

float prng_randf(void) {
    return prng_randf_r(&s_prng_state);
}

float prng_randf_norm_r(prng_state* rng) {
    if (!isnan(rng->prev_norm)) {
        float out = rng->prev_norm;
        rng->prev_norm = NAN;
        return out;
    }

    float u1 = 0.0f;
    do {
        u1 = prng_randf_r(rng);
    } while (u1 == 0.0f);

    float u2 = prng_randf_r(rng);

    float mag = sqrt(-2.0f * logf(u1));

    float z0 = mag * cosf(2.0 * PI * u2);
    float z1 = mag * sinf(2.0 * PI * u2);

    rng->prev_norm = z1;

    return z0;
}

float prng_randf_norm(void) {
    return prng_randf_norm_r(&s_prng_state);
}


// platform specific to macOS/unix
void plat_get_entropy(void* data, uint32_t size) {
    getentropy(data, size);
}
#include "pre_headers.h"
#include "random_generator.h"

std::random_device randDeivce;
std::default_random_engine random_generator::engine(randDeivce());

random_generator::random_generator()
{
}

random_generator::~random_generator()
{
}

int random_generator::get_random_int(int min, int max)
{
    std::uniform_int_distribution<int> random(min, max);

    return random(engine);
}
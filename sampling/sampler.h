/**
 * @file sampler.h
 * @author kubanec
 * @date 19.6.2013
 *
 */

#ifndef SAMPLER_H_
#define SAMPLER_H_

#include "ch.hpp"

class sampler : public chibios_rt::EnhancedThread<256>
{
public:
	sampler();
	msg_t Main(void);
};

#endif /* SAMPLER_H_ */

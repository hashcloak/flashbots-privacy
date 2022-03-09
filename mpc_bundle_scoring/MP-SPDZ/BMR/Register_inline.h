/*
 * Register_inline.h
 *
 */

#ifndef BMR_REGISTER_INLINE_H_
#define BMR_REGISTER_INLINE_H_

#include "CommonParty.h"
#include "Party.h"


inline Register ProgramRegister::new_reg()
{
	return Register(CommonParty::s().get_n_parties());
}

#endif /* BMR_REGISTER_INLINE_H_ */

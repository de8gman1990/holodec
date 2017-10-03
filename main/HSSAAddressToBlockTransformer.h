#ifndef HSSAADDRESSTOBLOCKTRANSFORMER_H
#define HSSAADDRESSTOBLOCKTRANSFORMER_H

#include "HSSATransformer.h"
namespace holodec {
	struct HSSAAddressToBlockTransformer : public HSSATransformParser {
		
		virtual void parseExpression (HSSAExpression* expression);
		
		virtual void parseBlock (HSSABB* block);

	};
}

#endif // HSSAADDRESSTOBLOCKTRANSFORMER_H

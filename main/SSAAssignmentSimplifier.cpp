#include "SSAAssignmentSimplifier.h"
#include "Argument.h"

#include "General.h"
#include "SSA.h"
#include "Function.h"


namespace holodec{
	
	void SSAAssignmentSimplifier::doTransformation (Function* function){
		
		printf ("Simplifying Assignments for Function at Address 0x%x\n", function->baseaddr);
		
		HMap<HId, SSAArgument> replacements;
		
		while(true){
			replacements.clear();
			for(SSAExpression& expr : function->ssaRep.expressions){
				if(!expr.id)
					continue;
				if(expr.type == SSAExprType::eAssign/* && !expr.subExpressions[0].isConst()*/) {
					
					SSAArgument arg = expr.subExpressions[0];
					if(arg.location == SSAExprLocation::eNone){
						if(expr.location != SSAExprLocation::eNone){
							arg.location = expr.location;
							arg.locref = expr.locref;
						}
					}
					replacements.insert(std::pair<HId, SSAArgument>(expr.id, arg));
				}else if(expr.type == SSAExprType::eUndef){
					replacements.insert(std::pair<HId, SSAArgument>(expr.id, SSAArgument::createUndef (expr.location, expr.locref, expr.size)));
				}else if(expr.type == SSAExprType::eLabel){
					replacements.insert(std::pair<HId, SSAArgument>(expr.id, SSAArgument::create()));
				}else if(expr.type == SSAExprType::ePhi) {
					bool undef = true;
					SSAArgument& firstArg = expr.subExpressions[0];
					bool alwaysTheSame = true;
					
					for(SSAArgument& arg : expr.subExpressions){
						if(arg.type != SSAArgType::eUndef){
							undef = false;
						}
						if(arg != firstArg){
							alwaysTheSame = false;
						}
					}
					if(undef){
						replacements.insert(std::pair<HId, SSAArgument>(expr.id, SSAArgument::createUndef (expr.location, expr.locref, expr.size)));
					}else if(alwaysTheSame){
						replacements.insert(std::pair<HId, SSAArgument>(expr.id, firstArg));
					}
				}else if(expr.type == SSAExprType::eSplit || expr.type == SSAExprType::eUpdatePart){
					if(expr.subExpressions[0].type == SSAArgType::eUndef){
						replacements.insert(std::pair<HId, SSAArgument>(expr.id, SSAArgument::createUndef(expr.location, expr.locref, expr.size)));
					}
				}else if(expr.type == SSAExprType::eLoadAddr){
					if(expr.subExpressions.size() == 5){
						SSAArgument &arg0 = expr.subExpressions[0], &arg1 = expr.subExpressions[1], &arg2 = expr.subExpressions[2], &arg3 = expr.subExpressions[3], &arg4 = expr.subExpressions[4];
						if(arg0.isValue(0)){
							if(arg1.isValue(0)){
								if(arg2.isValue(0)){
									replacements.insert(std::pair<HId, SSAArgument>(expr.id, arg4));
								}else if(arg3.isValue(0)){
									replacements.insert(std::pair<HId, SSAArgument>(expr.id, arg4));
								}
							}else if(arg4.isValue(0)){
								if(arg2.isValue(0)){
									replacements.insert(std::pair<HId, SSAArgument>(expr.id, arg1));
								}else if(arg3.isValue(0)){
									replacements.insert(std::pair<HId, SSAArgument>(expr.id, arg1));
								}
							}
						}
					}
				}else if(expr.type == SSAExprType::eReturn){
					for(auto it = expr.subExpressions.begin(); it != expr.subExpressions.end();){
						SSAArgument& arg = *it;
						SSAExprType type = function->ssaRep.expressions[arg.ssaId].type;
						while(arg.type == SSAArgType::eId && arg.ssaId && EXPR_IS_TRANSIENT(type)){
							arg = function->ssaRep.expressions[arg.ssaId].subExpressions[0];
							type = function->ssaRep.expressions[arg.ssaId].type;
						}
						if(arg.type == SSAArgType::eId && arg.ssaId && type == SSAExprType::eInput){
							it = expr.subExpressions.erase(it);
							continue;
						}
						++it;
					}
				}
			}
			if(replacements.empty())
				break;
			function->ssaRep.replaceNodes(&replacements);
			function->ssaRep.compress();
		}
	}
}
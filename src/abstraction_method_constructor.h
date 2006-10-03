/***************************************************************************
 *   Copyright (C) 2004 by Hiroyuki Kuwahara                               *
 *   kuwahara@cs.utah.edu                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#if !defined(HAVE_ABSTRACTION_METHOD_CONSTRUCTOR)
#define HAVE_ABSTRACTION_METHOD_CONSTRUCTOR

/*
    This header must only be included by abstraction_method_manager.h
    
    In order to add your new abstraction method, you just need to add your abstraction method constructor 
    in the __abstractionMethodConstrcutorTable.  Do not change abstraction method manager.
    
    this constructor type is used in all abstraction methods.

*/

typedef ABSTRACTION_METHOD * (*AbstractionMethodConstructorType)( ABSTRACTION_METHOD_MANAGER *manager );


extern ABSTRACTION_METHOD *DummyAbstractionMethodConstructor(  ABSTRACTION_METHOD_MANAGER *manager );
extern ABSTRACTION_METHOD *OpSiteBindingAbstractionMethodConstructor(  ABSTRACTION_METHOD_MANAGER *manager );
extern ABSTRACTION_METHOD *ModifierConstantPropagationAbstractionMethodConstructor(  ABSTRACTION_METHOD_MANAGER *manager );
extern ABSTRACTION_METHOD *ModifierStructureTransformationMethodConstructor(  ABSTRACTION_METHOD_MANAGER *manager );
extern ABSTRACTION_METHOD *ReversibleReactionStructureTransformationMethodConstructor(  ABSTRACTION_METHOD_MANAGER *manager );
extern ABSTRACTION_METHOD *NaryOrderUnaryTransformationMethodConstructor(  ABSTRACTION_METHOD_MANAGER *manager );
extern ABSTRACTION_METHOD *KineticLawConstantsSimplifierMethodConstructor(  ABSTRACTION_METHOD_MANAGER *manager );
extern ABSTRACTION_METHOD *EnzymeKineticRapidEquilibrium1MethodConstructor(  ABSTRACTION_METHOD_MANAGER *manager );
extern ABSTRACTION_METHOD *DimerToMonomerSubstitutionMethodConstructor(  ABSTRACTION_METHOD_MANAGER *manager );
extern ABSTRACTION_METHOD *KineticLawConstantsSimplifierMethodConstructor(  ABSTRACTION_METHOD_MANAGER *manager );
extern ABSTRACTION_METHOD *IrrelevantSpeciesEliminationMethodConstructor(  ABSTRACTION_METHOD_MANAGER *manager );
extern ABSTRACTION_METHOD *RnapOperatorBindingAbstractionMethodConstructor(  ABSTRACTION_METHOD_MANAGER *manager );
extern ABSTRACTION_METHOD *InducerStructureTransformationAbstractionMethodConstructor(  ABSTRACTION_METHOD_MANAGER *manager );
extern ABSTRACTION_METHOD *AbsoluteActivationInhibitionGenerationMethodConstructor(  ABSTRACTION_METHOD_MANAGER *manager );
extern ABSTRACTION_METHOD *FinalStateGenerationMethodConstructor(  ABSTRACTION_METHOD_MANAGER *manager ); 
extern ABSTRACTION_METHOD *SimilarReactionCombiningMethodConstructor(  ABSTRACTION_METHOD_MANAGER *manager );
extern ABSTRACTION_METHOD *AbsoluteInhibitionGenerationMethodConstructor(  ABSTRACTION_METHOD_MANAGER *manager );
extern ABSTRACTION_METHOD *ReversibleToIrreversibleMethodConstructor(  ABSTRACTION_METHOD_MANAGER *manager );
extern ABSTRACTION_METHOD *MultipleProductsReactionEliminationMethodConstructor(  ABSTRACTION_METHOD_MANAGER *manager );
extern ABSTRACTION_METHOD *MultipleReactantsReactionEliminationMethodConstructor(  ABSTRACTION_METHOD_MANAGER *manager ); 
extern ABSTRACTION_METHOD *SingleReactantProductReactionEliminationMethodConstructor(  ABSTRACTION_METHOD_MANAGER *manager );
extern ABSTRACTION_METHOD *StopFlagGenerationMethodConstructor(  ABSTRACTION_METHOD_MANAGER *manager );
extern ABSTRACTION_METHOD *EnzymeKineticQSSA1MethodConstructor(  ABSTRACTION_METHOD_MANAGER *manager );
extern ABSTRACTION_METHOD *DimerizationReductionMethodConstructor(  ABSTRACTION_METHOD_MANAGER *manager );
extern ABSTRACTION_METHOD *StoichiometryAmplificationMethodConstructor(  ABSTRACTION_METHOD_MANAGER *manager );
extern ABSTRACTION_METHOD *NaryOrderUnaryTransformationMethod2Constructor(  ABSTRACTION_METHOD_MANAGER *manager );
extern ABSTRACTION_METHOD *DegradationStoichiometryAmplificationMethodConstructor(  ABSTRACTION_METHOD_MANAGER *manager );
extern ABSTRACTION_METHOD *NaryOrderUnaryTransformationMethod3Constructor(  ABSTRACTION_METHOD_MANAGER *manager );
extern ABSTRACTION_METHOD *BirthDeathGenerationMethodConstructor(  ABSTRACTION_METHOD_MANAGER *manager );
extern ABSTRACTION_METHOD *DegradationStoichiometryAmplificationMethod2Constructor(  ABSTRACTION_METHOD_MANAGER *manager );
extern ABSTRACTION_METHOD *DegradationStoichiometryAmplificationMethod3Constructor(  ABSTRACTION_METHOD_MANAGER *manager );
extern ABSTRACTION_METHOD *BirthDeathGenerationMethod2Constructor(  ABSTRACTION_METHOD_MANAGER *manager );
extern ABSTRACTION_METHOD *BirthDeathGenerationMethod3Constructor(  ABSTRACTION_METHOD_MANAGER *manager );
extern ABSTRACTION_METHOD *DegradationStoichiometryAmplificationMethod4Constructor(  ABSTRACTION_METHOD_MANAGER *manager );
extern ABSTRACTION_METHOD *DegradationStoichiometryAmplificationMethod5Constructor(  ABSTRACTION_METHOD_MANAGER *manager );
extern ABSTRACTION_METHOD *DegradationStoichiometryAmplificationMethod6Constructor(  ABSTRACTION_METHOD_MANAGER *manager );
extern ABSTRACTION_METHOD *DegradationStoichiometryAmplificationMethod7Constructor(  ABSTRACTION_METHOD_MANAGER *manager );
extern ABSTRACTION_METHOD *DegradationStoichiometryAmplificationMethod8Constructor(  ABSTRACTION_METHOD_MANAGER *manager );
extern ABSTRACTION_METHOD *BirthDeathGenerationMethod4Constructor(  ABSTRACTION_METHOD_MANAGER *manager );
extern ABSTRACTION_METHOD *BirthDeathGenerationMethod5Constructor(  ABSTRACTION_METHOD_MANAGER *manager );
extern ABSTRACTION_METHOD *BirthDeathGenerationMethod6Constructor(  ABSTRACTION_METHOD_MANAGER *manager );
extern ABSTRACTION_METHOD *MaxConcentrationReactionAdditionAbstractionMethodConstructor(  ABSTRACTION_METHOD_MANAGER *manager );
extern ABSTRACTION_METHOD *StoichiometryAmplificationMethod2Constructor(  ABSTRACTION_METHOD_MANAGER *manager );
extern ABSTRACTION_METHOD *StoichiometryAmplificationMethod3Constructor(  ABSTRACTION_METHOD_MANAGER *manager );
extern ABSTRACTION_METHOD *BirthDeathGenerationMethod7Constructor(  ABSTRACTION_METHOD_MANAGER *manager );
extern ABSTRACTION_METHOD *EnzymeKineticRapidEquilibrium2MethodConstructor(  ABSTRACTION_METHOD_MANAGER *manager );
extern ABSTRACTION_METHOD *OpSiteBindingAbstractionMethod2Constructor(  ABSTRACTION_METHOD_MANAGER *manager );
extern ABSTRACTION_METHOD *PowKineticLawTransformationMethodConstructor(  ABSTRACTION_METHOD_MANAGER *manager );
extern ABSTRACTION_METHOD *DimerizationReductionLevelAssignmentConstructor(  ABSTRACTION_METHOD_MANAGER *manager );

static AbstractionMethodConstructorType __abstractionMethodConstrcutorTable[] = {
    DimerizationReductionLevelAssignmentConstructor,
    PowKineticLawTransformationMethodConstructor,
    OpSiteBindingAbstractionMethod2Constructor,
    EnzymeKineticRapidEquilibrium2MethodConstructor,
    BirthDeathGenerationMethod7Constructor, 
    StoichiometryAmplificationMethod3Constructor,
    StoichiometryAmplificationMethod2Constructor,
    MaxConcentrationReactionAdditionAbstractionMethodConstructor,
    BirthDeathGenerationMethod6Constructor,
    BirthDeathGenerationMethod5Constructor,
    BirthDeathGenerationMethod4Constructor,
    DegradationStoichiometryAmplificationMethod8Constructor,
    DegradationStoichiometryAmplificationMethod7Constructor,
    DegradationStoichiometryAmplificationMethod6Constructor,
    DegradationStoichiometryAmplificationMethod5Constructor,
    DegradationStoichiometryAmplificationMethod4Constructor, 
    BirthDeathGenerationMethod3Constructor, 
    BirthDeathGenerationMethod2Constructor,
    DegradationStoichiometryAmplificationMethod3Constructor,
    DegradationStoichiometryAmplificationMethod2Constructor,
    BirthDeathGenerationMethodConstructor,
    NaryOrderUnaryTransformationMethod3Constructor,
    DegradationStoichiometryAmplificationMethodConstructor,
    NaryOrderUnaryTransformationMethod2Constructor,
    StoichiometryAmplificationMethodConstructor,
    DimerizationReductionMethodConstructor,
    EnzymeKineticQSSA1MethodConstructor,
    StopFlagGenerationMethodConstructor,
    SingleReactantProductReactionEliminationMethodConstructor,
    MultipleReactantsReactionEliminationMethodConstructor,
    MultipleProductsReactionEliminationMethodConstructor,
    ReversibleToIrreversibleMethodConstructor,
    AbsoluteInhibitionGenerationMethodConstructor, 
    SimilarReactionCombiningMethodConstructor,
    FinalStateGenerationMethodConstructor,
    AbsoluteActivationInhibitionGenerationMethodConstructor, 
    InducerStructureTransformationAbstractionMethodConstructor,
    /* RnapOperatorBindingAbstractionMethodConstructor, */
    IrrelevantSpeciesEliminationMethodConstructor, 
    KineticLawConstantsSimplifierMethodConstructor,
    DimerToMonomerSubstitutionMethodConstructor,
    EnzymeKineticRapidEquilibrium1MethodConstructor,
    KineticLawConstantsSimplifierMethodConstructor, 
    NaryOrderUnaryTransformationMethodConstructor,
    ReversibleReactionStructureTransformationMethodConstructor,
    ModifierStructureTransformationMethodConstructor,
    ModifierConstantPropagationAbstractionMethodConstructor,
    OpSiteBindingAbstractionMethodConstructor,
    DummyAbstractionMethodConstructor,
    NULL
};

/*
#define ABSTRACTION_METHOD_MAX (0+1+1+1)
*/
#define ABSTRACTION_METHOD_MAX ((sizeof(__abstractionMethodConstrcutorTable)/sizeof(AbstractionMethodConstructorType))-1)


#endif

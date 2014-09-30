/*
 * LatentObject.cpp
 *
 *  Created on: Feb 9, 2014
 *      Author: qzhao2
 */

#include "LatentObject.h"

LatentObject::LatentObject(unsigned int id, unsigned int dim, bool isCovDiag)
:m_id(id)
,m_dim(dim)
,m_isCovDiag(isCovDiag){
	_init();
}

void LatentObject::_init(){
	/// initialize with Gaussian random numbers with mean 0 and variance 1
	m_posMean = colvec(m_dim,fill::randn);
	/// prior is always diagonal
	m_priorCov = mat(m_dim,1,fill::ones);
	if(m_isCovDiag){
		m_posCov = mat(m_dim,1,fill::ones);
	}else{
		m_posCov = mat(m_dim,m_dim);
		m_posCov.eye();
	}
}

/**
 * \brief inverse the posterior covariance matrix and multiply it with the
 * posterior mean vector to get m_posMeanInv
 *
 * @param toDist whether convert to distribution parameters
 */
void LatentObject::posInv(bool toDist){
	if(!toDist){
		if(m_isCovDiag){
			m_posCovInv = diagmat(1/m_posCov);
		}else{
			m_posCovInv = inv(m_posCov);
		}
		m_posMeanInv = m_posCovInv * m_posMean;
	}else{
		m_posCov = inv(m_posCovInv);
		m_posMean = m_posCov * m_posMeanInv;
		if(m_isCovDiag){
			m_posCov = m_posCov.diag();
		}
		/// reset the inverse variables
		m_posMeanInv = colvec();
		m_posCovInv = mat();
	}
}

LatentObject::~LatentObject() {
	// TODO Auto-generated destructor stub
}


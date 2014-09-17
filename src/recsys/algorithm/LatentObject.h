/*
 * LatentObject.h
 *
 *  Created on: Feb 9, 2014
 *      Author: qzhao2
 */

#ifndef LATENTOBJECT_H_
#define LATENTOBJECT_H_
#include <armadillo>

using namespace arma;
class LatentObject {
	friend class MFImplicitModel;
	friend class VLImplicitModel;
protected:
	unsigned int m_id;
	unsigned int m_dim; /// latent vector dimension
	colvec m_posMean; /// posterior mean
	colvec m_posMeanInv;
	colvec m_priorMean; // prior mean
	mat m_posCov; // posterior covariance
	mat m_posCovInv;
	mat m_priorCov; // prior covariance
	bool m_isCovDiag; /// whether covariance matrix is diagonal
public:
	LatentObject(unsigned int id, unsigned int dim, bool isCovDiag = true);
	void posInv(bool toDist = false);
	void reset(){
		m_posMean.fill(0);
		m_posMeanInv = m_posMean;
		m_posCov.fill(0);
		m_posCovInv = m_isCovDiag?diagmat(m_posCov):m_posCov;
	}
	virtual ~LatentObject();
protected:
	void _init();
};

#endif /* LATENTOBJECT_H_ */

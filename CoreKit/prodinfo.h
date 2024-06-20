/**
 * \file prodinfo.h
 * \brief Contains the macros required to embed product information into executable products.
 * \date 2013-11-18 10:16:00
 * \author Rolando J. Nieves
 */

#ifndef PRODINFO_H_
#define PRODINFO_H_

#define RF_PRODUCT_INFORMATION(ProductName, ProductVersion, BuildNumber, BuildDate, GitCommitId) \
static const char *ProductInformation = "$ProdInfo$" \
		"Name:"#ProductName \
		", Version:" ProductVersion \
		", Build:" BuildNumber \
		", Built on:" BuildDate \
		", Git Commit:" GitCommitId \
		; \
const char* ProductName##_product_information() { return ProductInformation; }


#endif /* PRODINFO_H_ */

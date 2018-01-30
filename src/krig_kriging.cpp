
#include "krig_kriging.h"


//--------------------------------------------------------------------------------------------------
arma::mat Kov( const arma::mat& X, 
               const arma::mat& Y, 
               Function Kern, 
               const bool symmetric ) {
  int i, j;
  int m = X.n_rows;
  int n = Y.n_rows;
  arma::mat K( n, m );
  arma::rowvec x, y;
  
  // KernPtr k = *XPtr< KernPtr >( Kern );
  
  // Filling Gaussian process covariance matrix
  if ( symmetric ) {
    for ( i = 0; i < n; i++ ) { 
      for ( j = i; j < n; j++ ) {
        x = X.row( i );
        y = Y.row( j );
        K( i, j ) = as<double>( Kern( x, y ) );
        
        if ( j > i ) {
          K( j, i ) = K( i, j );
        }
        
      }
    }
    
  } else { 
    for ( i = 0; i < n; i++ ) { 
      for ( j = 0; j < m; j++ ) {
        x = X.row( j );
        y = Y.row( i );
        K( i, j ) = as<double>( Kern( x, y ) );
      }
    }
  }
  
  return K;

}

//--------------------------------------------------------------------------------------------------
List Krig( const arma::mat& Z, 
           const arma::mat& K, 
           const arma::mat& k,
           const arma::mat& G, 
           const arma::mat& g,
           const std::string type, 
           const std::string cinv ) {

  int n = Z.n_rows;
  int m = k.n_cols;

  List KRIG;
  
  arma::mat J( n, n );
  arma::mat W( m, n );
  arma::mat L( m, n );

  // Inverse computation
  if ( cinv == "syminv" ) {
    J = inv_sympd( K );
    
  } else if ( cinv == "inv" ) {
    J = inv( K );
    
  } else if ( cinv == "cholinv" ) {
    J = chol( K );
    J = inv( J );
    J = J.t() * J;
    
  } else if ( cinv == "ginv" ) {
    J = K;
  }
  
  // Kriging computation
  if ( type == "simple" ) { // Simple kriging
    
    L = J *  k;
    W = L.t() * Z;  
    
    KRIG[ "Z" ] = W;
    KRIG[ "L" ] = L;
    KRIG[ "J" ] = J;
    
    
  } else if ( type == "ordinary" ) {  // Ordinary kriging
    double alpha;
    arma::mat u = arma::ones( n, 1 );
    arma::mat tau( n, m );

    alpha = 1.0 / as_scalar( u.t() * J * u );
    tau = arma::ones( n, m ) - arma::ones( n, n ) * J * k;
    
    L = J * ( k + alpha * tau ) ;
    W = L.t() * Z;
    
    KRIG[ "Z" ] = W;
    KRIG[ "L" ] = L;
    KRIG[ "J" ] = J;
    KRIG[ "alpha" ] = alpha;
    KRIG[ "tau" ] = tau;
    
  } else if ( type == "universal" ) { // Universal kriging
    
    int p = G.n_rows;
    arma::mat A( n, p );
    arma::mat tau( p, 1 );

    A = G.t() * inv_sympd( G * J * G.t() );
    tau = g - G * J * k;
    
    L = J * ( k + A * tau ) ;
    W = L.t() * Z;
    
    KRIG[ "Z" ] = W;
    KRIG[ "L" ] = L;
    KRIG[ "J" ] = J;
    KRIG[ "A" ] = A;
    KRIG[ "tau" ] = tau;

  }
  
  return KRIG;

}


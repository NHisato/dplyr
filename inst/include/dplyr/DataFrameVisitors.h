#ifndef dplyr_DataFrameVisitors_H
#define dplyr_DataFrameVisitors_H

namespace dplyr {

    class DataFrameVisitors : 
        public VisitorSetEqual<DataFrameVisitors>, 
        public VisitorSetHash<DataFrameVisitors>, 
        public VisitorSetLess<DataFrameVisitors>, 
        public VisitorSetGreater<DataFrameVisitors> {
        
        private:
            
            const Rcpp::DataFrame& data ;
            std::vector<VectorVisitor*> visitors ;
            Rcpp::CharacterVector visitor_names ;
            int nvisitors ;
            
        public:
            typedef VectorVisitor visitor_type ;
                                    
            DataFrameVisitors( const Rcpp::DataFrame& data_) :
                data(data_), 
                visitors(), 
                visitor_names(data.names()),
                nvisitors(visitor_names.size())
            {
                       
                for( int i=0; i<nvisitors; i++){
                    visitors.push_back( visitor( data[i] ) ) ;    
                }
            }
            DataFrameVisitors( const Rcpp::DataFrame& data_, const Rcpp::CharacterVector& names ) : 
                data(data_), 
                visitors(), 
                visitor_names(names), 
                nvisitors(visitor_names.size())
            {
                
                std::string name ;
                int n = names.size() ;
                for( int i=0; i<n; i++){
                    name = (String)names[i] ;
                    SEXP column ; 
                    
                    try{ 
                        column = data[name] ; 
                    } catch( ... ){
                        stop( "unknown column '%s' ", name ) ;
                    }
                    
                    visitors.push_back( visitor( column ) ) ;
                    
                }
                
            } 
            
            ~DataFrameVisitors(){
                delete_all( visitors );
            }     
            
            
            template <typename Container>
            DataFrame subset( const Container& index, const CharacterVector& classes ) const {
                List out(nvisitors);
                for( int k=0; k<nvisitors; k++){
                   out[k] = get(k)->subset(index) ;    
                }
                structure( out, Rf_length(out[0]) , classes) ;
                
                return (SEXP)out ;
            }
            
            inline int size() const { return nvisitors ; }
            inline VectorVisitor* get(int k) const { return visitors[k] ; }
            
            Rcpp::String name(int k) const { return visitor_names[k] ; }
            
            inline int nrows() const { return data.nrows() ; } 
            
        private:
            
            inline void structure( List& x, int nrows, CharacterVector classes ) const {
                x.attr( "class" ) = classes ;
                set_rownames(x, nrows) ;
                x.names() = visitor_names ;
                SEXP vars = data.attr( "vars" ) ;
                if( !Rf_isNull(vars) )
                    x.attr( "vars" ) = vars ;
            }
            
    } ;
          
    inline DataFrame subset( DataFrame data, LogicalVector test, CharacterVector select, CharacterVector classes ){
        DataFrameVisitors visitors( data, select ) ;
        return visitors.subset(test, classes ) ;
    }

    inline DataFrame subset( DataFrame data, LogicalVector test, CharacterVector classes ){
        DataFrameVisitors visitors( data ) ;
        return visitors.subset(test, classes ) ;
    }


} // namespace dplyr


#endif

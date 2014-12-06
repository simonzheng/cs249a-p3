#include <map>
#include <list>
#include <vector>

template< class Key, class Data > class Cache {
	private:
		typedef std::list< std::pair< Key, Data > > List; 
		typedef typename List::iterator ListIter;
		typedef std::map< Key, ListIter > Map;   
		typedef std::pair< Key, ListIter > Pair; 
		typedef typename Map::iterator MapIterator;	

	public:

		Cache( const unsigned long capacity ) :
				capacity_( capacity ),
				currentSize_( 0 )
				{ }

		~Cache() { clearAllData(); }

		
		void clearAllData( void ) {
			list_.clear();
			index_.clear();
		};

		inline bool containsCacheEntry( const Key &key ) const {
			return index_.find( key ) != index_.end();
		}

		inline void removeCacheEntry( const Key &key ) const {
			MapIterator miter = index_.find( key );
			if( miter == index_.end() ) return;
			removeCacheEntry_( miter );
		}

		inline void cacheEntryIs( const Key &key, const Data &data ) {
			MapIterator miter = bringToFront_( key );
			if( miter != index_.end() )
				removeCacheEntry_( miter );

			list_.push_front( std::make_pair( key, data ) );
			ListIter liter = list_.begin();

			index_.insert( std::make_pair( key, liter ) );
			currentSize_ += 1; 

			while( currentSize_ > capacity_ ) {
				liter = list_.end();
				--liter;
				removeCacheEntry_( liter->first );
			}
		}

		inline Data cacheEntry( const Key &key, bool bringToFront = true ) {
			MapIterator miter = index_.find( key );
			if( miter == index_.end() )
				return Data();
			Data tmp = miter->second->second;
			if( bringToFront )
				bringToFront_( key );
			return tmp;
		}

		inline bool cacheEntry( const Key &key, Data &data, bool bringToFront = true ) {
			MapIterator miter = index_.find( key );
			if( miter == index_.end() ) return false;
			if( bringToFront )
			  bringToFront_( key );
			data = miter->second->second;
			return true;
		}

	private:
		inline MapIterator bringToFront_( const Key &key ) {
			MapIterator miter = index_.find( key );
			if( miter == index_.end() ) return miter;
			list_.splice( list_.begin(), list_, miter->second );
			return miter;
		}

		inline void removeCacheEntry_( const MapIterator &miter ) {
			currentSize_ -= 1; 
			list_.erase( miter->second );
			index_.erase( miter );
		}

		inline void removeCacheEntry_( const Key &key ) {
			MapIterator miter = index_.find( key );
			removeCacheEntry_( miter );
		}

	private:		      
		List list_;       
		Map index_;       
		unsigned long capacity_;  
		unsigned long currentSize_; 
};

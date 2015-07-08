#include<vector>
#include<iostream>
#include <set>
using namespace std;


/* Center is actually a point */

template<class T> class Center {
	public:
		int K;		// dimenstions
		vector<T> coord; // four dim vector

		Center(int k, vector<T> &coord) {
			this->K = k;
			this->coord = coord;
		}
		Center(int k) {
			this->K = k;
		}
		bool operator ==(const Center &c) {
			if (this->K != c->K)
				return false;
			for (int i = 0; i < this->K; i++) {
				if (this->coord[i] != c->coord[i])
					return false;
			}
			return true;
		}
		bool check_dim(Center &c1, Center &c2) {
			return (c1->K == c2->K);
		}
		T compute_dist(Center &c2){
			T dist = 0;
			Center<T> c1 = *this;

			if (!check_dim(c1, c2)) {
				cout << "Cannot compare this centers\n";
				return new T;
			}
			for (int i = 0; i < c1.K; i++) { /* 1-norm distance */
				T diff = (c1.coord[i] - c2.coord[i]) * (c1.coord[i] - c2.coord[i]);
				dist += diff;
			}
			return sqrt(dist);
		}
};

template <class T>
class KMeans {
	vector<Center<T> > centers;
	vector<Center<T> > points;
	vector<set<Center<T> > > clusters;
	int K; /* clusters dimension */

	public:		/* init centers for kmeans algo */
		KMeans(int k, vector<Center<T> > c) {
			this->K = k;
			centers = c;
			// init clusters;
		}
		void assign_points() { // asign each point to a cluster based on min diff
			// Cleanup Sets
			for (int i = 0; i < clusters.size(); i++) {
				clusters[i].clear();
			}
			// Asign points
			for (int i = 0; i < points.size(); i++){
				Center<T> point = points[i];
				/* Default behaviour */
				int assign_center = 0;
				T min = point.compute_dist(centers[0]);
				/* Compute smallest distance */
				for (int i = 1; i < K; i++) {
					/* Compute difference with each cluster center */
					T dist = point.compute_distance(centers[i]);
					if (dist < min) {
						min = dist;
						assign_center = i;
					}
				}
				set<Center<T> > s = clusters[assign_center]; // shoud have dim 4
				s.insert(point);
			}
		}
		void init_centers() {
			for (int i = 0; i < K; i++) {
				
			}
		}
		void recompute_centers() {
			
		}
		void run() {
			// init
			// run until 
		}
};

int main() {
	return 1;
}


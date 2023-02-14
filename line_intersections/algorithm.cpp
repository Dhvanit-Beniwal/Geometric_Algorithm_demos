#include <iostream>
#include <set>
#include <vector>
#include <algorithm>
#include <string>
#include <cmath>
#include <limits>

constexpr double lowest_double = std::numeric_limits<double>::lowest();

using namespace std;

#define EPSILON 1e-5
#define CLOSE(a,b) (abs(a-b)<EPSILON)

/*  
    set<> does not look at the == operator for recognizing duplicates
    for set<>: a,b are the same iff ( !(a<b) && !(b<a) )
    but we can't rely on 2 points having their double variables == each other
    so both a<b and b<a must return false if a,b need to be recognised as equal.
*/


double sweep_line_y = lowest_double;    // y-coordinate of the sweep line

class point{
    public:
        double x,y;
        bool operator<(point rhs){
            point lhs = *this;
            return (lhs.y > rhs.y) || (CLOSE(lhs.y,rhs.y) && (lhs.x < rhs.x) && !CLOSE(lhs.x,rhs.x));
        }
        point operator-(point rhs){
            point lhs = *this;
            return point{lhs.x-rhs.x, lhs.y-rhs.y};
        }
        double y_x(){return y/x;}
        double x_y(){return x/y;}
        double x_D(){return x/(sqrt(x*x + y*y));}
};

class line{
    public:
        point lower,upper;
        line(double x0, double y0, double x1, double y1){
            if( (y0 > y1) || (CLOSE(y0,y1) && (x0 < x1)) ){
                upper = point{x0,y0};
                lower = point{x1,y1};
            }else{
                lower = point{x0,y0};
                upper = point{x1,y1};
            }
        }
        double rel_horz_pos(double x, double y){
            double &x0 = lower.x, &y0 = lower.y, &x1 = upper.x, &y1 = upper.y;
            if(CLOSE(y1,y0)){
                if(x < x1){return -1;}
                else if(x > x0){return 1;}
                else{return 0;}
            }
            return (x-x0) - (y-y0)*(x1-x0)/(y1-y0);
        }

        /*
            we would later want a point to traverse a set<> of lines just like a line would.
            (to find its neighbours)
            but we can't query a point object in a set<> of lines.
            so a line object with pseudo=true will represent a point 
            and be compared differently in the comparator function we write for this set<> of lines.
            as in, the comparator would encode both line-line and point-line comparisons.
        */
        bool pseudo = false; point pseudo_point;
        line(double x, double y){   // pseudo line's constructor, just a point
            pseudo = true;
            pseudo_point = point{x,y};
        }
};

class event{        
    public:
        point _point;
        vector < line* > U;
        vector < line* > L;
        bool operator<(const event &rhs)const{
            event lhs = *this;
            return lhs._point < rhs._point;
        }
        double x(){return _point.x;}
        double y(){return _point.y;}
};

struct intersection{
    double x;
    double y;
    vector < line* > lines;
};

// x-intercept of line given y
double x_intercept(line* lptr, double y){
    if(lptr->pseudo){return lptr->pseudo_point.x;} // assumed that y is same
    if (CLOSE(lptr->lower.y, lptr->upper.y)){return lptr->upper.x;}
    return (lptr->lower - lptr->upper).x_y()*(y-lptr->lower.y) + lptr->lower.x;
}

/*
    comparison according to angle with +x axis (between 0 and 180), 
    ordering of lines all intersecting on the sweep-line at the event point
*/
auto angle_cmp = [](line* a, line* b){   // assumed not pseudo
    double proj_a = (a->lower - a->upper).x_D();
    double proj_b = (b->lower - b->upper).x_D();
    if(!CLOSE(proj_a, proj_b)){
        return (proj_a < proj_b);
    }else{  // when lines overlap, not sure about this. arbirtary?
        if(!CLOSE(a->lower.y, b->lower.y)){return a->lower.y > b->lower.y;}else
        if(!CLOSE(a->upper.y, b->lower.y)){return a->upper.y > b->upper.y;}
        else{return false;}
    }
};

/*
    not an absolute ordering, depends on sweep-line's y coordinate (sweep_line_y). 
    It compares intercepts of lhs and rhs on the sweep line. 
    If they intersect on the sweep line, ordered according to the sweep line's future position (i.e. below)
    A pseudo line representing a point is assumed to be on the sweep line and ordered according to x-intercepts. 
    (pseudo-line < line) if the line intersects sweep-line at the pseudo-point.
    So lines.upper_bound(pseudo_point) will give the first member of L+C 
    does not expect (won't need) to compare two pseudo points
*/
auto line_comparator = [](line* a, line* b) {
    double y = sweep_line_y;
    if(a->pseudo){y = a->pseudo_point.y;}
    if(b->pseudo){y = b->pseudo_point.y;}
    double xa = x_intercept(a, y);
    double xb = x_intercept(b, y);
    if(!CLOSE(xa,xb)){
        return xa < xb;   
    }else{  // lines intersect on sweep-line.
        if(a->pseudo){return true;}
        if(b->pseudo){return false;}
        return angle_cmp(a,b);
    }
};

set< event > Q;     // event queue
std::set< line* , decltype(line_comparator)> T(line_comparator);    // segments currently on sweep-line
vector < line* > lines;     // all the lines in question
vector < intersection > intersections;      // all the intersections

// check if a valid intersection(l1,l2) is below p, if so: insert event to Q
void checkIntersection(line* l1, line* l2, point p){
    if(!l1 || !l2){return;}
    point p1 = l1->upper, p2 = l1->lower, p3 = l2->upper, p4 = l2->lower;
    if(CLOSE(p3.y, p4.y)){return;}
    
    event e{ point{} };
    
    if(CLOSE(p1.y, p2.y)){
        e._point.y = p1.y;
        double dist = l2->rel_horz_pos(p2.x,p2.y);
        if(dist < 0){return;}
        else{e._point.x = p2.x - dist;}
        Q.insert(e);
        return;
    }
    double m1 = (p1-p2).x_y(), m2 = (p3-p4).x_y();
    if(CLOSE(m1,m2)){return;}
    e._point.y = (m1*p1.y - p1.x + p3.x - m2*p3.y)/(m1-m2);
    e._point.x = p1.x + m1*(e._point.y-p1.y);
    if(e.y() > p.y || e.y() < p2.y || e.y() < p4.y){return;}
    Q.insert(e);
    return;
}

void handleEvent(event e){
    line* sl = nullptr;     // left neighbour 
    line* sr = nullptr;     // right neighbour
    line* s_ = nullptr;     // leftmost in U+C  (notation is s')
    line* s__ = nullptr;    // rightmost in U+C (notation is s")

    vector < line* > U,L,C;
    U = e.U;

    std::set< line* , decltype(angle_cmp)> U_C(angle_cmp);  // store U+C
    for(auto l : U){U_C.insert(l);}
    
    // find L,C
    line pseudo_line(e.x(), e.y());  // yea it's a hack, read constructor comment
    auto it = T.upper_bound(&pseudo_line);

    if(it!=T.begin()){--it; sl = *it; ++it;}
    while(it != T.end() && CLOSE((*it)->rel_horz_pos(e.x(), e.y()), 0)){
        bool is_in_L = CLOSE((*it)->lower.x, e.x()) && CLOSE((*it)->lower.y, e.y());
        if(is_in_L) {L.push_back(*it);}
        else        {C.push_back(*it); U_C.insert(*it);}
        ++it;
    }
    if(it!=T.end()){sr = *it;}
    if(U_C.size()){
        s_ = *U_C.begin();
        s__ = *U_C.rbegin();
    }
    
    // remove L, C
    for(auto lptr : L){T.erase(lptr);}
    for(auto lptr : C){T.erase(lptr);}
    // add U, C after updating sweep_line_y
    sweep_line_y =  e.y();
    for(auto lptr : U){T.insert(lptr);}
    for(auto lptr : C){T.insert(lptr);}

    if(U.size() + L.size() + C.size() >= 2){
        L.insert(L.end(), U.begin(), U.end()); // intersection lines
        L.insert(L.end(), C.begin(), C.end());
        intersections.push_back(intersection{e.x(), e.y(), L});
    }

    if((U.size() + C.size()) == 0){
        checkIntersection(sl, sr, e._point);
    }else{
        checkIntersection(sl, s_, e._point);
        checkIntersection(s__, sr, e._point);
    }

    return;
}

std::vector<intersection>& main_algo(std::vector< sf::Vertex >& sf_lines){
    // "sf_" stands for SFML.
    int N = sf_lines.size()/2;
    lines.reserve(N);
    for(int i=0; i<N; ++i){
        double x0,y0,x1,y1;
        x0 = sf_lines[2*i].position.x;
        y0 = sf_lines[2*i].position.y;
        x1 = sf_lines[2*i+1].position.x;
        y1 = sf_lines[2*i+1].position.y;
        lines.push_back( new line(x0,y0,x1,y1) );
        if(y0 > sweep_line_y){sweep_line_y = y0;}
        if(y1 > sweep_line_y){sweep_line_y = y1;}
    }
    // lines.push_back( new line(-2,0,2.5,2) );
    // lines.push_back( new line(0.8,-3,0,3) );
    // lines.push_back( new line(2.5,2,-4,1) );
    // lines.push_back( new line(-2,2,3,-3) );
    // sweep_line_y = 3;   // take max of all y, implement when reading input data from cin>>
    
    for (auto lineptr : lines){
        event e0{lineptr->lower};
        event e1{lineptr->upper};
        e0.L.push_back(lineptr);
        e1.U.push_back(lineptr);
        auto ret = Q.insert(e0);
        if(!ret.second){
            e0.L.insert(e0.L.end(), ret.first->L.begin(), ret.first->L.end());
            Q.erase(ret.first);
            Q.insert(e0);
        }
        ret = Q.insert(e1);
        if(!ret.second){
            e1.U.insert(e1.U.end(), ret.first->U.begin(), ret.first->U.end());
            Q.erase(ret.first);
            Q.insert(e1);
        }
    }
    while (! Q.empty()){
        event p = *(Q.begin());
        Q.erase(Q.begin());
        handleEvent(p);
    }

    // for(auto p : intersections){
    //     cout << "Intersection at: (" << p.x << ", " << p.y << ") of lines:" << endl;
    //     for(auto lptr : p.lines){
    //         point p1 = lptr->upper, p2 = lptr->lower;
    //         cout << "\t" << p1.x << " " << p1.y << " " << p2.x << " " << p2.y << endl;
    //     }
    // }
    for(auto lptr : lines){
        delete lptr; // free heap, since I don't intend to read intersection.lines in my SFML code anyway.
    }
    lines.clear();
    Q.clear();
    T.clear();
    return intersections;
}

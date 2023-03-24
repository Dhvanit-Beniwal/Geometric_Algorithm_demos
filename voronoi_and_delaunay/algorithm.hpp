#define INF std::numeric_limits<double>::max()
#define EPSILON 1e-5
#define IS_ZERO(a) (a<EPSILON && a>-EPSILON)
#define CLOSE(a,b) (IS_ZERO(a-b))

double sweep_line_y = -INF;    // y-coordinate of the sweeping line

class Point;
class Event;
class Arc;
class Edge;

std::vector< Point > points;
std::set< Event > Q;
std::set< Arc, std::less<> > T;
std::set< Edge > D;

bool show_delaunay = true;
bool show_voronoi = true;

class Point : public sf::Drawable{
        sf::CircleShape dot;
        void draw(sf::RenderTarget& target, sf::RenderStates states) const {
            target.draw(dot, states);
        }
    public:
        double x;
        double y;
        Point(double x, double y){
            this->x = x;
            this->y = y;

            //sfml
            float r = 2.f;
            dot = sf::CircleShape(r, 10);
            dot.setPosition(x-r,y-r);
            dot.setFillColor(sf::Color::White);
            dot.setOutlineThickness(2.f);
            dot.setOutlineColor(sf::Color::Blue);
        }
        bool operator< (const Point& rhs) const{
            if(!CLOSE(y, rhs.y)){
                return y > rhs.y;
            }else
            if(!CLOSE(x, rhs.x)){
                return x < rhs.x;
            }else{
                return false; // equal
            }
        }
        double intersect(const Point* rhsptr) const {
            // x-coord of intersection <lhs,rhs> of parabolas given by lhs and rhs (depends on sweeping line's position)
            // (different from <rhs,lhs> which is the other point of intersection of the same two parabolas)
            const Point& rhs = *rhsptr;

            if(CLOSE(y,rhs.y)){
                if(x<rhs.x){
                    return (x+rhs.x)/2;
                }else{
                    // undefined (unexpected?), should never get here
                    return INF;
                }
            }

            double x2 = x-rhs.x;
            double y2 = y-rhs.y;
            x2 *= x2; y2 *= y2;
            double y1_l = y-sweep_line_y;
            double y2_l = rhs.y-sweep_line_y;
            
            // why do i need to do this? because sqrt(-0.000) is -NaN. doubles are the bane of my existence. how is your day going, fellow comment reader?
            double K = (CLOSE(y1_l, 0) || CLOSE(y2_l, 0)) ? 0 : sqrt(y1_l*y2_l*(x2+y2));

            return (x*y2_l - rhs.x*y1_l + K)/(rhs.y-y);
            // turns out, the sign of (rhs.y-y) is exactly the choice of adding K vs subtracting K, so always +K (never -K)
        }
};

class Arc{
    public:
        Point* left; // maybe nullptr
        Point* middle;
        Point* right; // maybe nullptr
        
        /*
            an iterator of Q: 'pointer' to the circle event (if exists) where this arc disappears (Q.end() if doesn't exist)
            mutable means it can be changed for const objects (in place updating a std::set element)
        */
        mutable std::set< Event >::iterator circle_event;

        Arc(Point* a, Point* b, Point* c){
            left = a;
            middle = b;
            right = c;
            circle_event = Q.end();
        }

        double angle_measure() const {
            // used to compare zero width arcs (only?)
            // hard to explain in words without a diagram
            // chosen carefully but isn't an important part of the algorithm (i think)
            double angle1 = -atan2(middle->y-left->y, middle->x-left->x);
            double angle2 = -atan2(right->y-middle->y, right->x-middle->x);
            return (angle1+angle2)/2;
        }

        bool operator< (const Arc& rhs) const{
            if(!left){return true;}
            if(!right){return false;}
            if(!rhs.left){return false;}
            if(!rhs.right){return true;}

            double r1 = middle->intersect(right);
            double r2 = rhs.middle->intersect(rhs.right);
            if(!CLOSE(r1,r2)){
                return r1 < r2;
            }
            double l1 = left->intersect(middle);
            if(!CLOSE(l1,r1)){return true;}
            double l2 = rhs.left->intersect(rhs.middle);
            if(!CLOSE(l2,r2)){return false;}

            // co-incident and zero width arcs
            // if one of them is a degenerate parabola from a new site it comes before the other zero width arcs of non-degenerate parabolas
            if(CLOSE(middle->y, sweep_line_y)){
                return true;
            }
            if(CLOSE(rhs.middle->y, sweep_line_y)){
                return false;
            }
            // comparing actual zero width arcs (coinciding circle events cause this)
            return angle_measure()<rhs.angle_measure();
        }
        bool operator< (const double& x) const{
            if(!right){
                return false;
            }
            double tmp = middle->intersect(right);
            if(CLOSE(tmp, x)){
                return false;
            }
            return tmp < x;
        }
        friend bool operator< (const double& x, const Arc& rhs){
            if(!rhs.right){
                return true;
            }
            double tmp = rhs.middle->intersect(rhs.right);
            if(CLOSE(tmp, x)){
                return true;   
            }
            return x < tmp;
        }
};

class Event{
    public:
        double x;
        double y;
        bool isSite;

        // for site events only
        Point* site = nullptr;
        
        // for circle_events only 
        double y_c; // (y-cord of center = this->y + radius )
        std::set< Arc >::iterator arc; // an iterator of T: 'pointer' to the arc which disappears in this circle event

        Event(double x, double y, Point* p){
            this->x = x;
            this->y = y;
            site = p;
            isSite = true;
        }
        Event(double x, double y, double y_c, std::set< Arc >::iterator a){
            this->x = x;
            this->y = y;
            this->y_c = y_c;
            arc = a;
            isSite = false;
        }
        bool operator< (const Event& rhs) const{
            if(!CLOSE(y, rhs.y)){
                return y > rhs.y;
            }
            if(!CLOSE(x, rhs.x)){
                return x < rhs.x;
            }
            if(isSite && rhs.isSite){return false;} // two site events are duplicates
            
            if(isSite && !rhs.isSite){return false;} // circle before site
            if(!isSite && rhs.isSite){return true;} // circle before site
            
            return *arc < *(rhs.arc) ; // two circle events, order ambiguity deferred to their arcs
        }
};

class Edge : public sf::Drawable{
        Point* p1;
        Point* p2;
        mutable sf::Vertex sf_line[2];
        mutable bool finite = false;
        
        sf::Vertex del_edge[2]; // delaunay graph's edge

        void draw(sf::RenderTarget& target, sf::RenderStates states) const {
            if(show_voronoi){target.draw(sf_line, 2, sf::Lines, states);}
            if(show_delaunay){target.draw(del_edge, 2, sf::Lines, states);}
        }
    public:
        Edge(Point* p1, Point* p2, double x, double y, bool reverse = true){
            this->p1 = p1;
            this->p2 = p2;
            sf_line[0] = sf::Vertex(sf::Vector2f(x, y), sf::Color::White);
            double t = reverse ? -100 : 100;
            sf_line[1] = sf::Vertex(sf::Vector2f(x + t*(p2->y-p1->y), y - t*(p2->x-p1->x)), sf::Color::White);
            
            del_edge[0] = sf::Vertex(sf::Vector2f(p1->x, p1->y), sf::Color(0,255, 255, 50));
            del_edge[1] = sf::Vertex(sf::Vector2f(p2->x, p2->y), sf::Color(0,255, 255, 200));
        }
        void second_vertex(double x, double y) const {
            sf_line[1] = sf::Vertex(sf::Vector2f(x, y), sf::Color::White);
            finite = true;
        }
        bool operator<(const Edge& rhs) const {
            // .. idk what i'm doing,
            /*
                it would make so much more sense to use std::unordered_set 
                with overloaded operator== instead of this hot garbage 
                but then i need to define std::hash<Edge> 
                for an unordered pair of pointers and I just couldn't be bothered
                ... maybe some other day
            */
            auto& _p1 = std::min(p1,p2);
            auto& _p2 = std::max(p1,p2);
            auto& _rp1 = std::min(rhs.p1,rhs.p2);
            auto& _rp2 = std::max(rhs.p1,rhs.p2);
            return (_p1<_rp1) || (_p1==_rp1 && _p2<_rp2);
        }
};

void check_for_circle_event(std::set< Arc >::iterator self) {
    // self is the iterator in T for this object, to pass onto the circle_event we may create (whose iterator we save in this object)
    // storing pointers to each other would be easier but need it to point to the position in std::set
    auto& left = self->left;
    auto& middle = self->middle;
    auto& right = self->right;
    if(!left || !right){
        self->circle_event = Q.end(); 
        return;
    }
    if(left == right){
        self->circle_event = Q.end(); 
        return;
    }

    double x32 = right->x - middle->x, y32 = right->y - middle->y;
    double x21 = middle->x - left->x, y21 = middle->y - left->y;
    double cross = x32*y21 - y32*x21;
    if(CLOSE(cross, 0)){
        self->circle_event = Q.end();
        return;
    }

    double xb12 = left->intersect(middle);
    double xb23 = middle->intersect(right);
    //TODO to-do what happens when middle->y == sweep_line_y
    double yb12 = !CLOSE(y21,0) ? 
                        (left->y + middle->y)/2 - (x21/y21)*(xb12 - (left->x+middle->x)/2) :
                        (middle->y + sweep_line_y + (x21*x21)/(4*(middle->y - sweep_line_y)))/2;
    double yb23 = !CLOSE(y32,0) ?
                        (middle->y + right->y)/2 - (x32/y32)*(xb23 - (middle->x+right->x)/2) :
                        (middle->y + sweep_line_y + (x32*x32)/(4*(middle->y - sweep_line_y)))/2;
    double xb = xb23-xb12;
    double yb = yb23-yb12;

    // the direction in which a breakpoint <1,2> will grow is delX = (y2-y1) and delY = -(x2-x1) [with signs]
    // so the parameters t12 and t23 must be >=0 
    double t12 = (xb*x32+yb*y32)/cross;
    double t23 = (xb*x21+yb*y21)/cross;

    if(CLOSE(t12,0) && CLOSE(t23, 0) && CLOSE(middle->y, sweep_line_y)){
        // false alarm, breakpoints will actually diverge but start at the same point (which was reported by a circle event just before this)
        self->circle_event = Q.end();
        return;
    }
    if((t12 < 0 && !CLOSE(t12,0)) || (t23 < 0 && !CLOSE(t23, 0))){
        self->circle_event = Q.end();
        return;
    }

    double x = xb12 + t12*(y21);
    double y_c = yb12 - t12*(x21); // y-coord of center of circle
    double y = y_c - sqrt((x-middle->x)*(x-middle->x) + (y_c-middle->y)*(y_c-middle->y));
    self->circle_event = Q.insert(Event(x,y,y_c,self)).first;
    return;
}

void report_intersection(Arc arc, double x_c, double y_c){
    auto ret = D.insert(Edge(arc.left, arc.middle, x_c, y_c));
    if(!ret.second){
        ret.first->second_vertex(x_c, y_c);
    }
    ret = D.insert(Edge(arc.middle, arc.right, x_c, y_c));
    if(!ret.second){
        ret.first->second_vertex(x_c, y_c);
    }
    ret = D.insert(Edge(arc.left, arc.right, x_c, y_c, false));
    if(!ret.second){
        ret.first->second_vertex(x_c, y_c);
    }
}

void handleEvent(Event& event){
    sweep_line_y = event.y;
    if(event.isSite){
        // Site Event
        if(T.empty()){T.insert(Arc(nullptr, event.site, nullptr)); return;}
        auto it = T.upper_bound(event.x);
        auto arc = *it;
        
        if(arc.circle_event != Q.end()){
            Q.erase(arc.circle_event);
        }
        auto hint = std::next(it); // for O(1) inserts later
        T.erase(it);
        it = T.insert(hint, Arc(arc.left, arc.middle, event.site));
        check_for_circle_event(it);
        
        if(CLOSE(arc.middle->y, sweep_line_y)){
            T.insert(hint, Arc(arc.middle, event.site, arc.right));
        }else{
            T.insert(hint, Arc(arc.middle, event.site, arc.middle));
            it = T.insert(hint, Arc(event.site, arc.middle, arc.right));
            check_for_circle_event(it);
        }

        // new edge starts between event.site and arc.middle

    }else{
        // Circle event
        auto& it = event.arc;
        auto left_it = std::prev(it);
        auto right_it = std::next(it);
        auto hint = std::next(right_it);    // for O(1) inserts later
        if(left_it->circle_event != Q.end()){
            Q.erase(left_it->circle_event);
        }
        if(right_it->circle_event != Q.end()){
            Q.erase(right_it->circle_event);
        }
        
        auto left_arc = *left_it;
        auto right_arc = *right_it;

        auto arc = *it;
        double x_c = event.x;
        double y_c = event.y_c;

        T.erase(it);
        T.erase(left_it);
        T.erase(right_it);
        it = T.insert(hint, Arc(left_arc.left, left_arc.middle, right_arc.middle));
        check_for_circle_event(it);
        it = T.insert(hint, Arc(left_arc.middle, right_arc.middle, right_arc.right));
        check_for_circle_event(it);

        report_intersection(arc, x_c, y_c);
    }
}

void voronoi(){
    Q.clear();
    T.clear();
    D.clear();
    for(auto& p : points){
        Q.insert(Event(p.x, p.y, &p));
        if(sweep_line_y < p.y){sweep_line_y = p.y;}
    }
    while(!Q.empty()){
        auto it = Q.begin();
        Event event = *it;
        Q.erase(it);
        handleEvent(event);
    }
}
// Algorithm logic ends here

// Helper function to read from stdin. you're welcome
void read_from_stdin(int window_x, int window_y){
    int N;
    std::cout << "enter number of points: "; std::cin >> N;
    std::cout << "enter " << N << " lines of space separated coordinates" << std::endl;

    /*
        i smell a lack of comments, 
        "for you see it is not important for the main program, this is just a helper function my lad",
        said the programmer.
        "But what does it do?"
        "It helps me to not think about what it does by doing it"
    */
    std::vector< std::pair<double, double> >tmp(N);
    double x_max = -INF, x_min = INF, y_max = -INF, y_min = INF;
    for(int i=0; i<N; ++i){
        double x,y; std::cin >> x >> y;
        if(x>x_max){x_max = x;} if(x<x_min){x_min = x;}
        if(y>y_max){y_max = y;} if(y<y_min){y_min = y;}
        tmp[i] = std::pair<double,double>(x,y);
    }
    double del_y = y_max-y_min, del_x = x_max-x_min;
    if(del_x*window_y > del_y*window_x){
        double c = (del_x*window_y/window_x - del_y)/2;
        y_min -= c; y_max += c;
    }else
    if(del_x*window_y < del_y*window_x){
        double c = (del_y*window_x/window_y - del_x)/2;
        x_min -= c; x_max += c;
    }
    del_y = y_max-y_min, del_x = x_max-x_min;
    double xmargin = 0.5, ymargin = 0.5;
    y_max += del_y*ymargin; y_min -= del_y*ymargin; x_max += del_x*xmargin; x_min -= del_x*xmargin;
    double x_ = window_x/((1+2*xmargin)*del_x);
    double y_ = window_y/((1+2*ymargin)*del_y);
    points.reserve(N);
    for(auto& in: tmp){points.push_back(Point((in.first-x_min)*x_, (y_max-in.second)*y_));}
    // voronoi();
}
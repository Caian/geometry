#ifndef TESTS_RTREE_FUNCTION_HPP
#define TESTS_RTREE_FUNCTION_HPP

#include <boost/geometry/geometry.hpp>

#include <boost/geometry/extensions/index/rtree/rtree.hpp>
#include <boost/geometry/extensions/index/translator/index.hpp>
#include <boost/geometry/extensions/index/inserter.hpp>

#include <boost/geometry/extensions/index/rtree/visitors/print.hpp>

#include <boost/range/algorithm.hpp>
#include <boost/foreach.hpp>

#include <map>

namespace helpers
{

template <typename Indexable, size_t DI, typename Tag>
struct value_randomizer_impl_set {};

template <typename Box, size_t DI>
struct value_randomizer_impl_set<Box, DI, boost::geometry::box_tag>
{
    inline static void apply(
        Box & b,
        typename boost::geometry::index::traits::coordinate_type<Box>::type m,
        typename boost::geometry::index::traits::coordinate_type<Box>::type w)
    {
        namespace bg = boost::geometry;
        typedef typename bg::index::traits::coordinate_type<Box>::type coord_t;

        coord_t c1 = ::rand() / coord_t(RAND_MAX / m);
        coord_t c2 = ::rand() / coord_t(RAND_MAX / w);

        bg::set<bg::min_corner, DI>(b, c1 - c2);
        bg::set<bg::max_corner, DI>(b, c1 + c2);
    }
};

template <typename Point, size_t DI>
struct value_randomizer_impl_set<Point, DI, boost::geometry::point_tag>
{
    inline static void apply(
        Point & p,
        typename boost::geometry::index::traits::coordinate_type<Point>::type m,
        typename boost::geometry::index::traits::coordinate_type<Point>::type)
    {
        namespace bg = boost::geometry;
        typedef typename bg::index::traits::coordinate_type<Point>::type coord_t;

        coord_t c = ::rand() / coord_t(RAND_MAX / m);

        bg::set<DI>(p, c);
    }
};

template <typename Indexable, size_t D>
struct value_randomizer_impl
{
    inline static void apply(
        Indexable & i,
        typename boost::geometry::index::traits::coordinate_type<Indexable>::type m,
        typename boost::geometry::index::traits::coordinate_type<Indexable>::type w)
    {
        value_randomizer_impl<Indexable, D - 1>::apply(i, m, w);
        value_randomizer_impl_set<
            Indexable,
            D - 1,
            typename boost::geometry::index::traits::tag<Indexable>::type
        >::apply(i, m, w);
    }
};

template <typename Indexable>
struct value_randomizer_impl<Indexable, 1>
{
    inline static void apply(
        Indexable & i,
        typename boost::geometry::index::traits::coordinate_type<Indexable>::type m,
        typename boost::geometry::index::traits::coordinate_type<Indexable>::type w)
    {
        value_randomizer_impl_set<
            Indexable,
            0,
            typename boost::geometry::index::traits::tag<Indexable>::type
        >::apply(i, m, w);
    }
};

template <typename Indexable>
struct value_randomizer
{
    typedef Indexable value_type;

    typedef typename boost::geometry::index::traits::coordinate_type<Indexable>::type coord_t;
        
    inline value_randomizer(coord_t mm, coord_t ww)
        : m(mm), w(ww)
    {}

    inline Indexable operator()() const
    {
        namespace bg = boost::geometry;
        namespace bgi = bg::index;

        Indexable i;
        value_randomizer_impl<Indexable, bgi::traits::dimension<Indexable>::value>::apply(i, m, w);
        return i;
    }

    coord_t m, w;
};

template <typename Rtree, typename Cont, typename Randomizer>
void random_insert(Rtree & t, Cont & c, size_t n, Randomizer r)
{
    namespace bg = boost::geometry;
    namespace bgi = bg::index;

    bgi::insert_iterator<Rtree> ii = bgi::inserter(t);

    for ( size_t i = 0 ; i < n ; ++i )
    {
        typename Randomizer::value_type v = r();
        //bgi::insert(t, v);
        *ii++ = v;
        c.push_back(v);
    }
}

template <typename Cont, typename Translator>
bool results_compare(Cont const& c1, Cont const& c2, Translator const& tr)
{
    if ( c1.size() != c2.size() )
        return false;

    for ( typename Cont::const_iterator it = c1.begin() ; it != c1.end() ; ++it )
    {
        bool found = false;
        for ( typename Cont::const_iterator it2 = c2.begin() ; it2 != c2.end() ; ++it2 )
            if ( tr.equals(*it, *it2) )
            {
                found = true;
                break;
            }

            if ( !found )
                return false;
    }

    return true;
}

template <typename Point, typename Cont, typename Translator>
bool nearest_results_compare(Point const& p, Cont const& c1, Cont const& c2, Translator const& tr)
{
    namespace bg = boost::geometry;
    namespace bgi = boost::geometry::index;

    typedef typename Translator::indexable_type indexable_type;
    typedef bg::default_distance_result<Point, indexable_type>::type distance_type;

    if ( c1.size() != c2.size() )
        return false;

    if ( c1.size() == 0 && c2.size() == 0 )
        return true;

    distance_type biggest_distance1 = 0;

    for ( typename Cont::const_iterator it = c1.begin() ; it != c1.end() ; ++it )
    {
        distance_type curr_distance = bgi::comparable_distance_near(p, tr(*it));

        if ( biggest_distance1 < curr_distance )
            biggest_distance1 = curr_distance;
    }

    distance_type biggest_distance2 = 0;
    for ( typename Cont::const_iterator it = c2.begin() ; it != c2.end() ; ++it )
    {
        distance_type curr_distance = bgi::comparable_distance_near(p, tr(*it));

        if ( biggest_distance2 < curr_distance )
            biggest_distance2 = curr_distance;
    }

    return biggest_distance1 == biggest_distance2;
}

template <typename Point, typename Translator>
struct val_mindist_cmp
{
    val_mindist_cmp(Point const& p, Translator const& t)
        : pt(p), tr(t) 
    {}

    template <typename Value>
    bool operator()(Value const& v1, Value const& v2)
    {
        return boost::geometry::index::comparable_distance_near(pt, tr(v1))
            < boost::geometry::index::comparable_distance_near(pt, tr(v2));
    }

    Point const& pt;
    Translator const& tr;
};

template <typename Box, typename Iter, typename Translator>
Box values_box(Iter first, Iter last, Translator const& tr)
{
    namespace bg = boost::geometry;
    namespace bgi = bg::index;

    Box b;
    bg::assign_inverse(b);

    for ( ; first != last ; ++first )
    {
        bg::expand(b, tr(*first));
    }

    return b;
}

} // namespace helpers

template <typename Predicate, typename Rtree, typename Cont, typename Randomizer>
void random_query_check(Rtree const& t, Rtree const& t_copy, Cont const& c, size_t n, Randomizer r)
{
    namespace bg = boost::geometry;
    namespace bgi = bg::index;

    for ( size_t i = 0 ; i < n ; ++i )
    {
        Predicate pred = Predicate(r());

        std::vector<typename Rtree::value_type> res1, res2, res3;

        bgi::query(t, pred, std::back_inserter(res1));
        bgi::query(t_copy, pred, std::back_inserter(res2));

        for ( typename Cont::const_iterator it = c.begin() ; it != c.end() ; ++it )
        {
            if ( bgi::predicates_check<bgi::detail::rtree::value_tag>(pred, *it, t.translator()(*it)) )
                res3.push_back(*it);
        }

        std::stringstream ss;
        ss << "\nPredicate: " << typeid(Predicate).name() << "\n"
            << "res1: " << res1.size()
            << ", res2: " << res2.size()
            << ", res3: " << res3.size() << '\n';

        BOOST_CHECK_MESSAGE( helpers::results_compare(res1, res2, t.translator()), ss.str());
        BOOST_CHECK_MESSAGE( helpers::results_compare(res1, res3, t.translator()), ss.str());
    }
}

template <typename Predicate, typename Rtree, typename Cont, typename PointRandomizer, typename PredicateRandomizer>
void random_nearest_check(
    Rtree const& t,
    Rtree const& t_copy,
    Cont const& c,
    size_t n,
    PointRandomizer const& pr,
    size_t k,
    PredicateRandomizer const& r)
{
    namespace bg = boost::geometry;
    namespace bgi = bg::index;

    for ( size_t i = 0 ; i < n ; ++i )
    {
        typename PointRandomizer::value_type pt = pr();
        Predicate pred = Predicate(r());

        std::vector<typename Rtree::value_type> res1, res2, res3;

        bgi::nearest(t, pt, k, pred, std::back_inserter(res1));

        bgi::nearest(t_copy, pt, k, pred, std::back_inserter(res2));

        for ( typename Cont::const_iterator it = c.begin() ; it != c.end() ; ++it )
        {
            if ( bgi::predicates_check<bgi::detail::rtree::value_tag>(pred, *it, t.translator()(*it)) )
                res3.push_back(*it);
        }
        std::sort(
            res3.begin(),
            res3.end(),
            helpers::val_mindist_cmp<
                typename PointRandomizer::value_type,
                typename Rtree::translator_type
            >(pt, t.translator())
        );
        if ( k < res3.size() )
            res3.resize(k);

        std::stringstream ss;
        ss << "\nPredicate: " << typeid(Predicate).name() << "\n"
            << "res1: " << res1.size()
            << ", res2: " << res2.size()
            << ", res3: " << res3.size() << '\n';

        BOOST_CHECK_MESSAGE(helpers::nearest_results_compare(pt, res1, res2, t.translator()), ss.str());
        BOOST_CHECK_MESSAGE(helpers::nearest_results_compare(pt, res1, res3, t.translator()), ss.str());
    }
}

template <typename P, typename B, typename Tag>
struct tests_rtree_function_queries {};

template <typename P, typename B>
struct tests_rtree_function_queries<P, B, boost::geometry::point_tag>
{
    template <typename Rtree, typename Cont>
    inline static void apply(Rtree const& t, Rtree const& t_copy, Cont const& v)
    {
        namespace bgi = boost::geometry::index;

        random_query_check<B>(t, t_copy, v, 5, helpers::value_randomizer<B>(10, 5));
        random_query_check<bgi::detail::covered_by<B> >(t, t_copy, v, 5, helpers::value_randomizer<B>(10, 5));
        random_query_check<bgi::detail::disjoint<B> >(t, t_copy, v, 5, helpers::value_randomizer<B>(10, 5));
        random_query_check<bgi::detail::intersects<B> >(t, t_copy, v, 5, helpers::value_randomizer<B>(10, 5));
        random_query_check<bgi::detail::within<B> >(t, t_copy, v, 5, helpers::value_randomizer<B>(10, 5));
        random_query_check<bgi::detail::not_covered_by<B> >(t, t_copy, v, 5, helpers::value_randomizer<B>(10, 5));
        random_query_check<bgi::detail::not_disjoint<B> >(t, t_copy, v, 5, helpers::value_randomizer<B>(10, 5));
        random_query_check<bgi::detail::not_intersects<B> >(t, t_copy, v, 5, helpers::value_randomizer<B>(10, 5));
        random_query_check<bgi::detail::not_within<B> >(t, t_copy, v, 5, helpers::value_randomizer<B>(10, 5));

        random_nearest_check<bgi::detail::empty>(t, t_copy, v, 5, helpers::value_randomizer<P>(10, 0), 3, bgi::empty);
        random_nearest_check<B>(t, t_copy, v, 5, helpers::value_randomizer<P>(10, 0), 3, helpers::value_randomizer<B>(10, 5));
        random_nearest_check<bgi::detail::intersects<B> >(t, t_copy, v, 5, helpers::value_randomizer<P>(10, 0), 3, helpers::value_randomizer<B>(10, 5));
        random_nearest_check<bgi::detail::within<B> >(t, t_copy, v, 5, helpers::value_randomizer<P>(10, 0), 3, helpers::value_randomizer<B>(10, 5));
        random_nearest_check<bgi::detail::covered_by<B> >(t, t_copy, v, 5, helpers::value_randomizer<P>(10, 0), 3, helpers::value_randomizer<B>(10, 5));
        random_nearest_check<bgi::detail::disjoint<B> >(t, t_copy, v, 5, helpers::value_randomizer<P>(10, 0), 3, helpers::value_randomizer<B>(10, 5));
    }
};

template <typename P, typename B>
struct tests_rtree_function_queries<P, B, boost::geometry::box_tag>
{
    template <typename Rtree, typename Cont>
    inline static void apply(Rtree const& t, Rtree const& t_copy, Cont const& v)
    {
        namespace bgi = boost::geometry::index;

        random_query_check<B>(t, t_copy, v, 5, helpers::value_randomizer<B>(10, 5));
        random_query_check<bgi::detail::covered_by<B> >(t, t_copy, v, 5, helpers::value_randomizer<B>(10, 5));
        random_query_check<bgi::detail::disjoint<B> >(t, t_copy, v, 5, helpers::value_randomizer<B>(10, 5));
        random_query_check<bgi::detail::intersects<B> >(t, t_copy, v, 5, helpers::value_randomizer<B>(10, 5));
        random_query_check<bgi::detail::overlaps<B> >(t, t_copy, v, 5, helpers::value_randomizer<B>(10, 5));
        random_query_check<bgi::detail::within<B> >(t, t_copy, v, 5, helpers::value_randomizer<B>(10, 5));
        random_query_check<bgi::detail::not_covered_by<B> >(t, t_copy, v, 5, helpers::value_randomizer<B>(10, 5));
        random_query_check<bgi::detail::not_disjoint<B> >(t, t_copy, v, 5, helpers::value_randomizer<B>(10, 5));
        random_query_check<bgi::detail::not_intersects<B> >(t, t_copy, v, 5, helpers::value_randomizer<B>(10, 5));
        random_query_check<bgi::detail::not_overlaps<B> >(t, t_copy, v, 5, helpers::value_randomizer<B>(10, 5));
        random_query_check<bgi::detail::not_within<B> >(t, t_copy, v, 5, helpers::value_randomizer<B>(10, 5));

        random_nearest_check<bgi::detail::empty>(t, t_copy, v, 5, helpers::value_randomizer<P>(10, 0), 3, bgi::empty);
        random_nearest_check<B>(t, t_copy, v, 5, helpers::value_randomizer<P>(10, 0), 3, helpers::value_randomizer<B>(10, 5));
        random_nearest_check<bgi::detail::intersects<B> >(t, t_copy, v, 5, helpers::value_randomizer<P>(10, 0), 3, helpers::value_randomizer<B>(10, 5));
        random_nearest_check<bgi::detail::overlaps<B> >(t, t_copy, v, 5, helpers::value_randomizer<P>(10, 0), 3, helpers::value_randomizer<B>(10, 5));
        random_nearest_check<bgi::detail::within<B> >(t, t_copy, v, 5, helpers::value_randomizer<P>(10, 0), 3, helpers::value_randomizer<B>(10, 5));
        random_nearest_check<bgi::detail::covered_by<B> >(t, t_copy, v, 5, helpers::value_randomizer<P>(10, 0), 3, helpers::value_randomizer<B>(10, 5));
        random_nearest_check<bgi::detail::disjoint<B> >(t, t_copy, v, 5, helpers::value_randomizer<P>(10, 0), 3, helpers::value_randomizer<B>(10, 5));
    }
};

template <typename Value, typename Options, typename Translator>
void tests_rtree_function(Translator const& tr = Translator())
{
    namespace bg = boost::geometry;
    namespace bgi = bg::index;

    bgi::rtree<Value, Options, Translator> t(tr);
    std::vector<Value> v;

    typedef typename bgi::rtree<Value, Options, Translator>::indexable_type I;
    typedef typename bgi::rtree<Value, Options, Translator>::box_type B;
    typedef typename bgi::traits::point_type<B>::type P ;

    helpers::random_insert(t, v, 10, helpers::value_randomizer<Value>(10, 1));

    bgi::rtree<Value, Options, Translator> t_copy(t);

    BOOST_CHECK(bgi::size(t) == 10);
    BOOST_CHECK(bgi::size(t) == bgi::size(t_copy));
    BOOST_CHECK(bg::equals(bgi::box(t), bgi::box(t_copy)));

    B bt = bgi::box(t);
    B bv = helpers::values_box<B>(v.begin(), v.end(), tr);
    BOOST_CHECK(bg::equals(bt, bv));

    tests_rtree_function_queries<P, B, bgi::traits::tag<I>::type>::apply(t, t_copy, v);

    bgi::clear(t);
    BOOST_CHECK(bgi::empty(t));
    bt = bgi::box(t);
    B be;
    bg::assign_inverse(be);
    BOOST_CHECK(bg::equals(be, bt));
}

BOOST_AUTO_TEST_CASE(tests_rtree_function_box3f)
{
#ifdef TEST_PRINT_INFO
    std::cout << "tests/rtree_function_box3f\n";
#endif

    namespace bg = boost::geometry;
    namespace bgi = bg::index;

    typedef bg::model::point<float, 3, bg::cs::cartesian> P;
    typedef bg::model::box<P> B;
    typedef B V;

    tests_rtree_function< V, bgi::linear<4, 2>, bgi::translator::def<V> >();
    tests_rtree_function< V, bgi::quadratic<4, 2>, bgi::translator::def<V> >();
    tests_rtree_function< V, bgi::rstar<4, 2>, bgi::translator::def<V> >();
}

BOOST_AUTO_TEST_CASE(tests_rtree_function_box2f)
{
#ifdef TEST_PRINT_INFO
    std::cout << "tests/rtree_function_box2f\n";
#endif

    namespace bg = boost::geometry;
    namespace bgi = bg::index;

    typedef bg::model::point<float, 2, bg::cs::cartesian> P;
    typedef bg::model::box<P> B;
    typedef B V;

    tests_rtree_function< V, bgi::linear<4, 2>, bgi::translator::def<V> >();
    tests_rtree_function< V, bgi::quadratic<4, 2>, bgi::translator::def<V> >();
    tests_rtree_function< V, bgi::rstar<4, 2>, bgi::translator::def<V> >();
}

BOOST_AUTO_TEST_CASE(tests_rtree_function_point2f)
{
#ifdef TEST_PRINT_INFO
    std::cout << "tests/rtree_function_point2f\n";
#endif

    namespace bg = boost::geometry;
    namespace bgi = bg::index;

    typedef bg::model::point<float, 2, bg::cs::cartesian> P;
    typedef P V;

    tests_rtree_function< V, bgi::linear<4, 2>, bgi::translator::def<V> >();
    tests_rtree_function< V, bgi::quadratic<4, 2>, bgi::translator::def<V> >();
    tests_rtree_function< V, bgi::rstar<4, 2>, bgi::translator::def<V> >();
}

namespace helpers {

template <typename Indexable>
struct value_randomizer< std::pair<Indexable, int> >
{
    typedef std::pair<Indexable, int> value_type;

    typedef typename boost::geometry::index::traits::coordinate_type<Indexable>::type coord_t;

    inline value_randomizer(coord_t mm, coord_t ww)
        : r(mm, ww)
    {}

    inline value_type operator()() const
    {
        return std::make_pair(r(), ::rand());
    }

    value_randomizer<Indexable> r;
};

} // namespace helpers

BOOST_AUTO_TEST_CASE(tests_rtree_function_pair_box2f_int)
{
#ifdef TEST_PRINT_INFO
    std::cout << "tests/rtree_function_pair_box2f_int\n";
#endif

    namespace bg = boost::geometry;
    namespace bgi = bg::index;

    typedef bg::model::point<float, 2, bg::cs::cartesian> P;
    typedef bg::model::box<P> B;
    typedef std::pair<B, int> V;

    tests_rtree_function< V, bgi::linear<4, 2>, bgi::translator::def<V> >();
    tests_rtree_function< V, bgi::quadratic<4, 2>, bgi::translator::def<V> >();
    tests_rtree_function< V, bgi::rstar<4, 2>, bgi::translator::def<V> >();
}

namespace helpers {

template <typename Indexable>
struct value_randomizer< boost::shared_ptr< std::pair<Indexable, int> > >
{
    typedef boost::shared_ptr< std::pair<Indexable, int> > value_type;

    typedef typename boost::geometry::index::traits::coordinate_type<Indexable>::type coord_t;

    inline value_randomizer(coord_t mm, coord_t ww)
        : r(mm, ww)
    {}

    inline value_type operator()() const
    {
        return value_type(new std::pair<Indexable, int>(r(), ::rand()));
    }

    value_randomizer<Indexable> r;
};

} // namespace helpers

BOOST_AUTO_TEST_CASE(tests_rtree_function_shared_ptr_pair_box2f_int)
{
#ifdef TEST_PRINT_INFO
    std::cout << "tests/rtree_function_shared_ptr_pair_box2f_int\n";
#endif

    namespace bg = boost::geometry;
    namespace bgi = bg::index;

    typedef bg::model::point<float, 2, bg::cs::cartesian> P;
    typedef bg::model::box<P> B;
    typedef boost::shared_ptr< std::pair<B, int> > V;

    tests_rtree_function< V, bgi::linear<4, 2>, bgi::translator::def<V> >();
    tests_rtree_function< V, bgi::quadratic<4, 2>, bgi::translator::def<V> >();
    tests_rtree_function< V, bgi::rstar<4, 2>, bgi::translator::def<V> >();
}

#endif // TESTS_RTREE_FUNCTION_HPP

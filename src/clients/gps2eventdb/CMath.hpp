#if ! defined( MATH_CLASS_HEADER_FILE )

#define MATH_CLASS_HEADER_FILE

#define HALF_OF_PI          1.5707963267948966
#define ONE_FOURTH_OF_PI    0.78539816339744833
#define PI                  3.14159265358979323846
#define TWO_TIMES_PI        6.2831853071795864769
#define RADIANS_TO_DEGREES 57.29577951308232
#define DEGREES_TO_RADIANS   .0174532925199432958

class CMath
{
   public:

      CMath();
      virtual ~CMath();

      double Cosine(  const double value );
      double Tangent( const double value );
};

#endif // EARTH_CLASS_HEADER_FILE

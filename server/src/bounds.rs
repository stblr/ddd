pub trait Bounds {
    const MIN: Self;
    const MAX: Self;
}

impl<T: Copy + Bounds, const N: usize> Bounds for [T; N] {
    const MIN: Self = [T::MIN; _];
    const MAX: Self = [T::MAX; _];
}

macro_rules! impl_bounds {
    ($t:ty) => {
        impl Bounds for $t {
            const MIN: Self = Self::MIN;
            const MAX: Self = Self::MAX;
        }
    };
}

impl_bounds!(i8);
impl_bounds!(i16);
impl_bounds!(i32);
impl_bounds!(i64);
impl_bounds!(i128);
impl_bounds!(u8);
impl_bounds!(u16);
impl_bounds!(u32);
impl_bounds!(u64);
impl_bounds!(u128);
impl_bounds!(f32);
impl_bounds!(f64);

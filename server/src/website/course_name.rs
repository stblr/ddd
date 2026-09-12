use std::fmt::{Result, Write};

use crate::base64;
use crate::courses::Courses;
use crate::website::html::Children;

pub fn write(
    courses: &Courses,
    course_hash: &[u8; 32],
    parent: &mut Children<impl Write>,
) -> Result {
    let mut a = parent.element("a")?;
    let course = base64::display(course_hash);
    a.attribute("href")?.value(format_args!("../courses/{course}"))?;
    match courses.get(course_hash) {
        Some(course) => a.content(course),
        None => a.content(format_args!("{course:.12}...")),
    }
}

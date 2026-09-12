use std::fmt::{Display, Result, Write};

use crate::website::html::{Children, Element};

pub fn write(
    name: impl Display,
    write: impl FnOnce(&mut Children<String>) -> Result,
    page: &mut String,
) -> Result {
    "<!doctype html>\n".clone_into(page);
    write_html(name, write, page)
}

fn write_html<W: Write>(
    name: impl Display,
    write: impl FnOnce(&mut Children<W>) -> Result,
    page: &mut W,
) -> Result {
    let mut html = Element::new(page, 0, "html")?;
    html.attribute("lang")?.value("en-US")?;
    let mut html = html.children()?;
    write_head(&name, &mut html)?;
    write_body(&name, write, &mut html)?;
    html.finish()
}

fn write_head(name: impl Display, html: &mut Children<impl Write>) -> Result {
    let mut head = html.element("head")?.children()?;
    write_meta("viewport", "width=device-width, initial-scale=1", &mut head)?;
    write_meta("color-scheme", "light dark", &mut head)?;
    write_title(format_args!("{name} · Double Dash Deluxe"), &mut head)?;
    write_link("stylesheet", "../../../data/style.css", &mut head)?;
    head.finish()
}

fn write_meta(name: &str, content: &str, head: &mut Children<impl Write>) -> Result {
    let mut meta = head.element("meta")?;
    meta.attribute("name")?.value(name)?;
    meta.attribute("content")?.value(content)?;
    meta.empty()
}

fn write_title(title: impl Display, head: &mut Children<impl Write>) -> Result {
    head.element("title")?.content(title)
}

fn write_link(rel: &str, href: &str, head: &mut Children<impl Write>) -> Result {
    let mut meta = head.element("link")?;
    meta.attribute("rel")?.value(rel)?;
    meta.attribute("href")?.value(href)?;
    meta.empty()
}

fn write_body<W: Write>(
    name: impl Display,
    write: impl FnOnce(&mut Children<W>) -> Result,
    html: &mut Children<W>,
) -> Result {
    let mut body = html.element("body")?.children()?;
    body.element("h1")?.content(name)?;
    write(&mut body)?;
    body.finish()
}

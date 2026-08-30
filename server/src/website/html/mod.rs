use std::fmt::{Display, Error, Write};
use std::result;

pub struct Element<'a, W: Write> {
    writer: &'a mut W,
    indent: usize,
    tag: &'static str,
}

impl<'a, W: Write> Element<'a, W> {
    pub fn new(writer: &'a mut W, indent: usize, tag: &'static str) -> Result<Self> {
        write!(writer, "<{tag}")?;
        Ok(Self { writer, indent, tag })
    }

    pub fn attribute(&mut self, name: &str) -> Result<Attribute<'_, W>> {
        Attribute::new(self.writer, name)
    }

    pub fn empty(self) -> Result<()> {
        write!(self.writer, ">")
    }

    pub fn content(self, content: impl Display) -> Result<()> {
        write!(self.writer, ">{content}</{}>", self.tag)
    }

    pub fn children(self) -> Result<Children<'a, W>> {
        write!(self.writer, ">")?;
        Ok(Children::new(self))
    }
}

impl<W: Write> Drop for Element<'_, W> {
    fn drop(&mut self) {}
}

pub struct Attribute<'a, W: Write> {
    writer: &'a mut W,
}

impl<'a, W: Write> Attribute<'a, W> {
    fn new(writer: &'a mut W, name: &str) -> Result<Self> {
        write!(writer, " {name}")?;
        Ok(Self { writer })
    }

    pub fn empty(self) {}

    pub fn value(self, value: &str) -> Result<()> {
        write!(self.writer, "=\"{value}\"")
    }
}

impl<W: Write> Drop for Attribute<'_, W> {
    fn drop(&mut self) {}
}

pub struct Children<'a, W: Write> {
    parent: Element<'a, W>,
}

impl<'a, W: Write> Children<'a, W> {
    fn new(parent: Element<'a, W>) -> Self {
        Self { parent }
    }

    pub fn element(&mut self, tag: &'static str) -> Result<Element<'_, W>> {
        self.line(self.parent.indent + 1)?;
        Element::new(self.parent.writer, self.parent.indent + 1, tag)
    }

    fn content(&mut self, content: impl Display) -> Result<()> {
        self.line(self.parent.indent + 1)?;
        write!(self.parent.writer, "{content}")
    }

    pub fn finish(mut self) -> Result<()> {
        self.line(self.parent.indent)?;
        write!(self.parent.writer, "</{}>", self.parent.tag)
    }

    fn line(&mut self, indent: usize) -> Result<()> {
        writeln!(self.parent.writer)?;
        for _ in 0..indent {
            write!(self.parent.writer, "    ")?;
        }
        Ok(())
    }
}

impl<W: Write> Drop for Children<'_, W> {
    fn drop(&mut self) {}
}

pub type Result<T> = result::Result<T, Error>;

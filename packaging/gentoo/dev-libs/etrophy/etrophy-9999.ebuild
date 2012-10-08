# Copyright 1999-2010 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

EAPI="2"
ESVN_SUB_PROJECT="PROTO"
inherit enlightenment

DESCRIPTION="EFL trophy library"

RDEPEND=">=media-libs/elementary-9999"
DEPEND="${RDEPEND}"

src_configure() {
	enlightenment_src_configure
}

import { Title } from "@solidjs/meta"
import { Component } from "solid-js"

import Reader from "~/components/reader";

const Index: Component = () => {
  return (
    <>
      <Title>Guided Reader</Title>
      <Reader />
    </>
  );
};

export default Index;
import { Component, JSX } from "solid-js";
import styles from "./annotation-list.module.css";

interface AnnotationListProps {
  class?: string;
  children: JSX.Element;
}

const AnnotationList: Component<AnnotationListProps> = (props) => {
  const { class: class_ = "", children } = props;

  return <div class={`${styles.annotation_list} ${class_}`}>{children}</div>;
};

export default AnnotationList;

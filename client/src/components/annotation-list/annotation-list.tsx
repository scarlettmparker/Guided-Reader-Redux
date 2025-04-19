import { Component, JSX } from "solid-js";
import { Card, CardBody, CardHeader } from "~/components/card";
import HideIcon from "~/components/hide-icon";
import styles from "./annotation-list.module.css";

interface AnnotationListProps {
  class?: string;
  children: JSX.Element;
}

const AnnotationList: Component<AnnotationListProps> = (props) => {
  const { class: class_ = "", children } = props;

  return (
    <Card class={`${styles.annotation_list} ${class_}`}>
      <CardHeader class={styles.annotation_list_header}>
        <HideIcon reverse={true} class={styles.hide_icon}>
          X
        </HideIcon>
        <span>Annotations</span>
      </CardHeader>
      <CardBody class={styles.annotation_list_body}>{children}</CardBody>
    </Card>
  );
};

export default AnnotationList;

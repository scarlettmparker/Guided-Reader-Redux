import { Component } from "solid-js";
import { Card, CardBody, CardFooter, CardHeader } from "~/components/card";
import LikeButton from "~/components/like-button";
import { AnnotationResponse } from "~/types";
import styles from "./annotation.module.css";

interface AnnotationProps {
  annotation: AnnotationResponse;
}

const Annotation: Component<AnnotationProps> = (props) => {
  const { annotation } = props;
  const author = annotation.author;

  return (
    <Card>
      <CardHeader class={styles.annotation_header}>
        <span class={styles.username}>{author.username}</span>
        <span class={styles.annotation_date}>1 month ago</span>
      </CardHeader>
      <CardBody>{annotation.description}</CardBody>
      <CardFooter class={styles.annotation_footer}>
        <LikeButton class={styles.like_button} />
        <span class={styles.likes_count}>
          {annotation.likes - annotation.dislikes}
        </span>
        <LikeButton reverse={true} />
      </CardFooter>
    </Card>
  );
};

export default Annotation;

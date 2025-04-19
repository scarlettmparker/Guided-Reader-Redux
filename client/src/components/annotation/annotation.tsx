import { Component } from "solid-js";
import { Card, CardBody, CardFooter, CardHeader } from "~/components/card";
import LikeButton from "~/components/like-button";
import ProfilePicture from "~/components/profile-picture";
import { unixToDate } from "~/utils/annotation";
import { AnnotationResponse } from "~/types";
import styles from "./annotation.module.css";

interface AnnotationProps {
  annotation: AnnotationResponse;
}

const Annotation: Component<AnnotationProps> = (props) => {
  const { annotation } = props;
  const author = annotation.author;

  return (
    <Card class={styles.annotation}>
      <a href={`/user/${author.discord_id}`}>
        <CardHeader class={styles.annotation_header}>
          <ProfilePicture
            avatar={author.avatar}
            discord_id={author.discord_id}
            class={styles.profile_picture}
          />
          <span class={styles.username}>{author.username}</span>
          <span class={styles.annotation_date}>
            {unixToDate(annotation.created_at)}
          </span>
        </CardHeader>
      </a>
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

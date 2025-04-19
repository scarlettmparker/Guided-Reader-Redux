import { Component, createEffect, createSignal, onMount } from "solid-js";
import { Card, CardBody, CardFooter, CardHeader } from "~/components/card";
import LikeButton from "~/components/like-button";
import ProfilePicture from "~/components/profile-picture";
import { unixToDate } from "~/utils/annotation";
import { AnnotationResponse } from "~/types";
import { marked } from "marked";
import styles from "./annotation.module.css";

interface AnnotationProps {
  annotation: AnnotationResponse;
}

const Annotation: Component<AnnotationProps> = (props) => {
  const { annotation } = props;
  const author = annotation.author;
  const [markedDescription, setMarkedDescription] = createSignal<string>("");

  onMount(() => {
    const renderer = new marked.Renderer();

    // Add target="_blank" to all links to open in new tab
    renderer.link = function ({ href, title, text }) {
      const target = "_blank";
      const rel = "noopener noreferrer";
      return `<a href="${href}" title="${title || ""}" target="${target}" rel="${rel}">${text}</a>`;
    };

    marked.setOptions({
      breaks: true,
      renderer: renderer,
    });
  });

  createEffect(async () => {
    setMarkedDescription(await marked(annotation.description));
  });

  return (
    <Card class={styles.annotation}>
      <a href={`/user/${author.discord_id}`} class={styles.annotation_link}>
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
      <CardBody>
        <div
          class={styles.annotation_description}
          innerHTML={markedDescription()}
        />
      </CardBody>
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
